/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ecclesia/lib/redfish/raw.h"

#include <cstdint>
#include <memory>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/types/optional.h"
#include "ecclesia/lib/redfish/interface.h"
#include "ecclesia/lib/redfish/test_mockup.h"

namespace libredfish {
namespace {

using testing::Eq;

TEST(RawInterfaceTestWithMockup, GetUriMatchesGetRoot) {
  TestingMockupServer mockup("indus_hmb_cn_mockup.shar");
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
  TestingMockupServer mockup("indus_hmb_cn_mockup.shar");
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
  TestingMockupServer mockup("indus_hmb_cn_mockup.shar");
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
  TestingMockupServer mockup("indus_hmb_cn_mockup.shar");
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

TEST(RawInterfaceTestWithMockup, PostUri) {
  TestingMockupServer mockup("indus_hmb_cn_mockup.shar");
  auto raw_intf = mockup.RedfishClientInterface();
  auto origin_collection = raw_intf->GetUri("/redfish/v1/Chassis").AsIterable();
  ASSERT_TRUE(origin_collection);
  auto origin_size = origin_collection->Size();

  auto res = raw_intf
                 ->PostUri("/redfish/v1/Chassis",
                           {
                               {"key1", 1},
                               {"key2", 1.3},
                               {"key3", "test"},
                               {"key4", true},
                           })
                 .AsObject();
  // After propagate status code to RedfishInvariant, add test to verify
  // status code for different type of resources. For example, for resource
  // support Action, return 204, others return 404.
  // For 204 response, there is no payload. So this will return empty
  // RedfishInvariant. The following ASSERT will fail.
  // ASSERT_TRUE(res);

  auto new_collection = raw_intf->GetUri("/redfish/v1/Chassis").AsIterable();
  ASSERT_TRUE(origin_collection);
  auto new_size = new_collection->Size();
  EXPECT_EQ(new_size - origin_size, 1);
  auto new_chassis = new_collection->GetIndex(new_size - 1).AsObject();

  EXPECT_EQ(new_chassis->GetNodeValue<int32_t>("key1").value_or(0), 1);
  EXPECT_EQ(new_chassis->GetNodeValue<double>("key2").value_or(0.0), 1.3);
  EXPECT_EQ(new_chassis->GetNodeValue<std::string>("key3").value_or(""),
            "test");
  EXPECT_EQ(new_chassis->GetNodeValue<bool>("key4").value_or(false), true);
}

}  // namespace
}  // namespace libredfish
