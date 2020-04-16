// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// These set of tests verify that the MachineMaster service interface adheres to
// the structure and patterns outlined by the master API design for modelling
// resources.

#include <stddef.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include "base/logging.h"
#include "net/proto2/public/descriptor.h"
#include "net/proto2/public/message.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "mmaster/service/identifiers.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {
namespace {

using ::testing::Contains;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NotNull;

// Constants for the package and service names.
constexpr absl::string_view kPackage = "ecclesia";
constexpr absl::string_view kService = "MachineMasterService";

// Verify that there are no resources name which are a prefix of another name.
// This ensures that we can never have clashes between the names of RPCs or
// types for two different resources.
TEST(ServiceApiTest, NoResourcesArePrefixesOfAnother) {
  // If one string in the list was a prefix of another, sorting them would place
  // the prefix string immediately before one of the ones it was a prefix of.
  std::vector<absl::string_view> resources(std::cbegin(kServiceResourceTypes),
                                           std::cend(kServiceResourceTypes));
  std::sort(resources.begin(), resources.end());
  for (size_t i = 1; i < resources.size(); ++i) {
    EXPECT_THAT(absl::StartsWith(resources[i], resources[i - 1]), IsFalse());
  }
}

// Top-level test to verify that the set of RPCs is exactly what is expected.
// This can't be done in a resource-specific test because this needs to check
// for RPCs that aren't part of a resource.
TEST(ServiceApiTest, AllRpcsAreValid) {
  // Fetch the service descriptor.
  const ::proto2::ServiceDescriptor *service = ABSL_DIE_IF_NULL(
      ::proto2::DescriptorPool::generated_pool()->FindServiceByName(
          absl::StrFormat("%s.%s", kPackage, kService)));

  // Build up a set of expected enumerate and query RPCs.
  absl::flat_hash_set<std::string> expected_rpcs;
  for (absl::string_view resource : kServiceResourceTypes) {
    expected_rpcs.insert(absl::StrFormat("Enumerate%s", resource));
    expected_rpcs.insert(absl::StrFormat("Query%s", resource));
  }
  for (const ResourceVerb &resource_verb : kResourceVerbs) {
    expected_rpcs.insert(
        absl::StrCat("Mutate", resource_verb.resource, resource_verb.verb));
  }

  // Iterate over the complete set of RPCs and complain about any that are not
  // in the expected set. We don't do the reverse check, that every RPC in the
  // expected set is defined, since that will already be covered by the
  // resource-specific checks.
  for (int i = 0; i < service->method_count(); ++i) {
    const ::proto2::MethodDescriptor *method = service->method(i);
    const std::string &name = method->name();
    EXPECT_THAT(expected_rpcs, Contains(name));
  }
}

class ResourceApiTest : public ::testing::TestWithParam<absl::string_view> {
 public:
  static std::string PrintParamName(
      ::testing::TestParamInfo<absl::string_view> param_info) {
    return std::string(param_info.param);
  }

  ResourceApiTest()
      : resource_(GetParam()),
        pool_(::proto2::DescriptorPool::generated_pool()),
        service_(ABSL_DIE_IF_NULL(pool_->FindServiceByName(
            absl::StrFormat("%s.%s", kPackage, kService)))) {
    // Ensure that the metadata for the service does not get optimized out by
    // the linker. Otherwise the test might fail.
    proto2::LinkMessageReflection<GetProcessInfoResponse>();
  }

 protected:
  absl::string_view resource_;

  const ::proto2::DescriptorPool *pool_;
  const ::proto2::ServiceDescriptor *service_;
};

TEST_P(ResourceApiTest, IdentifierType) {
  // There must be an identifier type.
  std::string id_type_name =
      absl::StrFormat("%s.%sIdentifier", kPackage, resource_);
  const auto *id_type = pool_->FindMessageTypeByName(id_type_name);
  EXPECT_THAT(id_type, NotNull());
}

TEST_P(ResourceApiTest, EnumerateRpc) {
  // There must be an enumerate RPC.
  std::string rpc_name = absl::StrFormat("Enumerate%s", resource_);
  const auto *rpc = service_->FindMethodByName(rpc_name);
  ASSERT_THAT(rpc, NotNull());

  // The client sends a single request, the server provides multiple responses.
  EXPECT_THAT(rpc->client_streaming(), IsFalse());
  EXPECT_THAT(rpc->server_streaming(), IsTrue());

  // The request message is empty.
  EXPECT_THAT(rpc->input_type()->full_name(), Eq("google.protobuf.Empty"));

  // The response type must have an "id" field of the identifier type.
  const auto *response_type = rpc->output_type();
  EXPECT_THAT(
      response_type->full_name(),
      Eq(absl::StrFormat("%s.Enumerate%sResponse", kPackage, resource_)));
  const auto *id_field = response_type->FindFieldByName("id");
  ASSERT_THAT(id_field, NotNull());
  const auto *id_field_type = id_field->message_type();
  ASSERT_THAT(id_field_type, NotNull());
  EXPECT_THAT(id_field_type->full_name(),
              Eq(absl::StrFormat("%s.%sIdentifier", kPackage, resource_)));
}

TEST_P(ResourceApiTest, QueryRpc) {
  // There must be a query RPC.
  std::string rpc_name = absl::StrFormat("Query%s", resource_);
  const auto *rpc = service_->FindMethodByName(rpc_name);
  ASSERT_THAT(rpc, NotNull());

  // The client can send multiple queries and the server provides multiple
  // responses.
  EXPECT_THAT(rpc->client_streaming(), IsTrue());
  EXPECT_THAT(rpc->server_streaming(), IsTrue());

  // The request type must have an "id" field of the identifier type and must
  // have a "field_mask" field for selecting response fields.
  {
    const auto *request_type = rpc->input_type();
    EXPECT_THAT(request_type->full_name(),
                Eq(absl::StrFormat("%s.Query%sRequest", kPackage, resource_)));
    const auto *id_field = request_type->FindFieldByName("id");
    ASSERT_THAT(id_field, NotNull());
    const auto *id_field_type = id_field->message_type();
    ASSERT_THAT(id_field_type, NotNull());
    EXPECT_THAT(id_field_type->full_name(),
                Eq(absl::StrFormat("%s.%sIdentifier", kPackage, resource_)));
    const auto mask_field = request_type->FindFieldByName("field_mask");
    ASSERT_THAT(mask_field, NotNull());
    const auto *mask_field_type = mask_field->message_type();
    ASSERT_THAT(mask_field_type, NotNull());
    EXPECT_THAT(mask_field_type->full_name(), Eq("google.protobuf.FieldMask"));
  }

  // The response type must have an "id" field of the identifier type and must
  // have a "status" field indicating the validity of the particular response.
  {
    const auto *response_type = rpc->output_type();
    EXPECT_THAT(response_type->full_name(),
                Eq(absl::StrFormat("%s.Query%sResponse", kPackage, resource_)));
    const auto *id_field = response_type->FindFieldByName("id");
    ASSERT_THAT(id_field, NotNull());
    const auto *id_field_type = id_field->message_type();
    ASSERT_THAT(id_field_type, NotNull());
    EXPECT_THAT(id_field_type->full_name(),
                Eq(absl::StrFormat("%s.%sIdentifier", kPackage, resource_)));
    const auto status_mask = response_type->FindFieldByName("status");
    ASSERT_THAT(status_mask, NotNull());
    const auto *status_mask_type = status_mask->message_type();
    ASSERT_THAT(status_mask_type, NotNull());
    EXPECT_THAT(status_mask_type->full_name(), Eq("google.rpc.Status"));
  }
}

TEST_P(ResourceApiTest, MutateRpc) {
  for (const ResourceVerb &rv : kResourceVerbs) {
    if (rv.resource != resource_) continue;
    const auto *rpc = service_->FindMethodByName(
        absl::StrCat("Mutate", rv.resource, rv.verb));
    ASSERT_THAT(rpc, NotNull());

    // The client can send single queries and the server a single response.
    EXPECT_THAT(rpc->client_streaming(), IsFalse());
    EXPECT_THAT(rpc->server_streaming(), IsFalse());

    // The request type must have an "id" field of the identifier type
    {
      const auto *request_type = rpc->input_type();
      EXPECT_THAT(request_type->full_name(),
                  Eq(absl::StrFormat("%s.%sRequest", kPackage, rpc->name())));
      const auto *id_field = request_type->FindFieldByName("id");
      ASSERT_THAT(id_field, NotNull());
      const auto *id_field_type = id_field->message_type();
      ASSERT_THAT(id_field_type, NotNull());
      EXPECT_THAT(id_field_type->full_name(),
                  Eq(absl::StrFormat("%s.%sIdentifier", kPackage, resource_)));
    }

    // The response type does not need to have an "id" field for Mutate
    {
      const auto *response_type = rpc->output_type();
      EXPECT_THAT(response_type->full_name(),
                  Eq(absl::StrFormat("%s.%sResponse", kPackage, rpc->name())));
    }
  }
}

INSTANTIATE_TEST_SUITE_P(AllResources, ResourceApiTest,
                         ::testing::ValuesIn(kServiceResourceTypes),
                         ResourceApiTest::PrintParamName);

}  // namespace
}  // namespace ecclesia
