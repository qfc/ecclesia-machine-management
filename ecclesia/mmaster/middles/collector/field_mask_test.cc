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

#include "mmaster/middles/collector/field_mask.h"

#include "google/protobuf/field_mask.proto.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {
namespace {

using ::testing::EqualsProto;

TEST(FieldMaskTest, TrimToNothing) {
  ::google::protobuf::FieldMask empty_mask;

  QueryFirmwareResponse qfr;
  qfr.mutable_id()->set_devpath("/phys");
  qfr.set_model("abc");
  qfr.set_version("1.2.3");
  qfr.set_build_date("2010-05-02");
  qfr.set_updateable(true);

  TrimResponseMessage(empty_mask, &qfr);
  EXPECT_THAT(qfr, EqualsProto(""));
}

TEST(FieldMaskTest, TrimToPartial) {
  ::google::protobuf::FieldMask partial_mask;
  partial_mask.add_paths("id");
  partial_mask.add_paths("model");
  partial_mask.add_paths("version");

  QueryFirmwareResponse qfr;
  qfr.mutable_id()->set_devpath("/phys");
  qfr.set_model("abc");
  qfr.set_version("1.2.3");
  qfr.set_build_date("2010-05-02");
  qfr.set_updateable(true);

  TrimResponseMessage(partial_mask, &qfr);
  EXPECT_THAT(qfr, EqualsProto(R"pb(
                id { devpath: "/phys" }
                model: "abc"
                version: "1.2.3"
              )pb"));
}

TEST(FieldMaskTest, TrimToFull) {
  ::google::protobuf::FieldMask full_mask;
  full_mask.add_paths("id");
  full_mask.add_paths("model");
  full_mask.add_paths("version");
  full_mask.add_paths("build_date");
  full_mask.add_paths("updateable");

  QueryFirmwareResponse qfr;
  qfr.mutable_id()->set_devpath("/phys");
  qfr.set_model("abc");
  qfr.set_version("1.2.3");
  qfr.set_build_date("2010-05-02");
  qfr.set_updateable(true);

  TrimResponseMessage(full_mask, &qfr);
  EXPECT_THAT(qfr, EqualsProto(R"pb(
                id { devpath: "/phys" }
                model: "abc"
                version: "1.2.3"
                build_date: "2010-05-02"
                updateable: true
              )pb"));
}

TEST(FieldMaskTest, DoesFieldMaskHaveAnyOf) {
  ::google::protobuf::FieldMask mask;
  mask.add_paths("model");
  mask.add_paths("version");

  // Check single fields in the mask.
  EXPECT_TRUE(DoesFieldMaskHaveAnyOf(mask, "model"));
  EXPECT_TRUE(DoesFieldMaskHaveAnyOf(mask, "version"));

  // Check single fields not in the mask.
  EXPECT_FALSE(DoesFieldMaskHaveAnyOf(mask, "build_date"));
  EXPECT_FALSE(DoesFieldMaskHaveAnyOf(mask, "updateable"));

  // Check multiple fields, subset of the mask.
  EXPECT_TRUE(DoesFieldMaskHaveAnyOf(mask, "model"));
  EXPECT_TRUE(DoesFieldMaskHaveAnyOf(mask, "version"));
  EXPECT_TRUE(DoesFieldMaskHaveAnyOf(mask, "model", "version"));

  // Check multiple fields, some but not all are in the mask.
  EXPECT_TRUE(
      DoesFieldMaskHaveAnyOf(mask, "model", "build_date", "updateable"));
  EXPECT_TRUE(DoesFieldMaskHaveAnyOf(mask, "version", "updateable"));

  // Check multiple fields, none are in the mask.
  EXPECT_FALSE(DoesFieldMaskHaveAnyOf(mask, "build_date", "updateable"));
}

}  // namespace
}  // namespace ecclesia
