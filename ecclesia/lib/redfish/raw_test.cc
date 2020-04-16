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

#include "lib/redfish/raw.h"

#include <memory>
#include <optional>
#include <string>

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "lib/redfish/interface.h"
#include "lib/redfish/test_mockup.h"

namespace libredfish {
namespace {

using testing::Eq;

TEST(RawInterfaceTestWithMockup, GetUriMatchesGetRoot) {
  TestingMockupServer mockup(
      "google3/redfish_mockups/indus_hmb_cn_mockup.sar");
  auto raw_intf = mockup.RedfishClientInterface();
  auto root = raw_intf->GetRoot().AsObject();
  auto root_via_uri = raw_intf->GetUri("/redfish/v1").AsObject();
  auto root_via_uri2 = raw_intf->GetUri("/redfish/v1/").AsObject();
  ASSERT_TRUE(root);
  ASSERT_TRUE(root_via_uri);
  ASSERT_TRUE(root_via_uri2);
  EXPECT_THAT(root->GetUri(), Eq(root_via_uri->GetUri()));
  EXPECT_THAT(root->GetUri(), Eq(root_via_uri2->GetUri()));
}

TEST(RawInterfaceTestWithMockup, GetChildObjectByUriMatches) {
  TestingMockupServer mockup(
      "google3/redfish_mockups/indus_hmb_cn_mockup.sar");
  auto raw_intf = mockup.RedfishClientInterface();
  auto root = raw_intf->GetRoot().AsObject();
  ASSERT_TRUE(root);
  auto chassis = root->GetNode("Chassis").AsObject();
  ASSERT_TRUE(chassis);

  auto chassis_via_uri = raw_intf->GetUri("/redfish/v1/Chassis").AsObject();
  ASSERT_TRUE(chassis_via_uri);

  EXPECT_THAT(chassis->GetUri(), Eq(chassis_via_uri->GetUri()));
}

TEST(RawInterfaceTestWithMockup, GetIndusObjectByUriMatches) {
  TestingMockupServer mockup(
      "google3/redfish_mockups/indus_hmb_cn_mockup.sar");
  auto raw_intf = mockup.RedfishClientInterface();
  auto root = raw_intf->GetRoot().AsObject();
  ASSERT_TRUE(root);
  auto chassis = root->GetNode("Chassis").AsIterable();
  ASSERT_TRUE(chassis);
  auto indus = chassis->GetIndex(0).AsObject();
  ASSERT_TRUE(indus);

  auto indus_via_uri = raw_intf->GetUri("/redfish/v1/Chassis/chassis").AsObject();
  ASSERT_TRUE(indus_via_uri);

  EXPECT_THAT(indus->GetUri(), Eq(indus_via_uri->GetUri()));
}

TEST(RawInterfaceTestWithMockup, GetFragmentUriMatches) {
  TestingMockupServer mockup(
      "google3/redfish_mockups/indus_hmb_cn_mockup.sar");
  auto raw_intf = mockup.RedfishClientInterface();
  auto root = raw_intf->GetRoot().AsObject();
  ASSERT_TRUE(root);
  auto chassis = root->GetNode("Chassis").AsIterable();
  ASSERT_TRUE(chassis);
  auto indus = chassis->GetIndex(0).AsObject();
  ASSERT_TRUE(indus);
  auto indus_assembly = indus->GetNode("Assembly").AsObject();
  ASSERT_TRUE(indus_assembly);
  auto assemblies = indus_assembly->GetNode("Assemblies").AsIterable();
  ASSERT_TRUE(assemblies);
  auto assembly = assemblies->GetIndex(0).AsObject();
  ASSERT_TRUE(assembly);

  auto assembly_via_uri =
      raw_intf->GetUri("/redfish/v1/Chassis/chassis/Assembly#/Assemblies/0")
          .AsObject();
  ASSERT_TRUE(assembly_via_uri);

  EXPECT_THAT(assembly->GetUri(), Eq(assembly_via_uri->GetUri()));
}

TEST(FactoryTest, FailedConnectionReturnsNullInterface) {
  auto redfish_intf = NewRawInterface("bad_endpoint");
  ASSERT_TRUE(redfish_intf);
  auto root = redfish_intf->GetRoot();
  EXPECT_FALSE(root.AsObject());
  EXPECT_FALSE(root.AsIterable());
}

}  // namespace
}  // namespace libredfish
