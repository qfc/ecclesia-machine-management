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

#include "mmaster/backends/redfish/redfish.h"

#include <string>
#include <utility>
#include <vector>

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "lib/redfish/property.h"
#include "lib/redfish/property_definitions.h"
#include "lib/redfish/result.h"
#include "lib/redfish/test_mockup.h"
#include "mmaster/config/config.proto.h"

namespace ecclesia {
namespace {

using IntResult = libredfish::Result<int>;
using StringResult = libredfish::Result<std::string>;
using ::testing::UnorderedElementsAre;

AgentConfig AgentConfigFromMockup(
    const libredfish::TestingMockupServer &mockup) {
  auto mockup_config = mockup.GetConfig();

  AgentConfig config;
  config.set_name("redfish");
  config.mutable_redfish()->set_hostname(mockup_config.hostname);
  config.mutable_redfish()->set_port(mockup_config.port);
  return config;
}

class RedfishBackendTestWithMockup : public ::testing::Test {
 protected:
  RedfishBackendTestWithMockup()
      : mockup_(
            "google3/redfish_mockups/"
            "indus_hmb_cn_mockup.sar"),
        backend_(AgentConfigFromMockup(mockup_)) {}

  // Helper for calling sysmodel->QueryAll and returning all results in an
  // array.
  template <typename PropertyDefinitionT>
  std::vector<libredfish::Result<typename PropertyDefinitionT::type>>
  BackendQueryAllToArray() {
    std::vector<libredfish::Result<typename PropertyDefinitionT::type>> results;
    backend_.QueryAll<PropertyDefinitionT>(
        [&results](auto r) { results.push_back(std::move(r)); });
    return std::move(results);
  }

  // Helper for calling sysmodel->Query and returning all results in an array.
  template <typename PropertyDefinitionT>
  std::vector<libredfish::Result<typename PropertyDefinitionT::type>>
  BackendQueryToArray(absl::Span<const absl::string_view> devpaths) {
    std::vector<libredfish::Result<typename PropertyDefinitionT::type>> results;
    backend_.Query<PropertyDefinitionT>(
        devpaths, [&results](auto r) { results.push_back(std::move(r)); });
    return std::move(results);
  }

 private:
  libredfish::TestingMockupServer mockup_;

 protected:
  RedfishBackend backend_;
};

TEST_F(RedfishBackendTestWithMockup, GetFrus) {
  EXPECT_THAT(
      backend_.GetDevpaths(),
      UnorderedElementsAre(
          "/phys", "/phys:connector:CPU0", "/phys:connector:CPU1",
          "/phys:connector:DIMM0", "/phys:connector:DIMM1",
          "/phys:connector:DIMM2", "/phys:connector:DIMM3",
          "/phys:connector:DIMM4", "/phys:connector:DIMM5",
          "/phys:connector:DIMM6", "/phys:connector:DIMM7",
          "/phys:connector:DIMM8", "/phys:connector:DIMM9",
          "/phys:connector:DIMM10", "/phys:connector:DIMM11",
          "/phys:connector:DIMM12", "/phys:connector:DIMM13",
          "/phys:connector:DIMM14", "/phys:connector:DIMM15",
          "/phys:connector:DIMM16", "/phys:connector:DIMM17",
          "/phys:connector:DIMM18", "/phys:connector:DIMM19",
          "/phys:connector:DIMM20", "/phys:connector:DIMM21",
          "/phys:connector:DIMM22", "/phys:connector:DIMM23",
          "/phys:connector:KA0", "/phys:connector:KA1", "/phys:connector:PE0",
          "/phys:connector:PE1", "/phys:connector:PE2", "/phys:connector:PE3",
          "/phys:connector:PE4", "/phys:connector:PE5", "/phys:connector:PE6",
          "/phys:connector:IPASS0", "/phys:connector:IPASS1",
          "/phys:connector:USB_lower", "/phys:connector:USB_upper",
          "/phys/CPU0", "/phys/CPU1", "/phys/DIMM0", "/phys/DIMM1",
          "/phys/DIMM2", "/phys/DIMM3", "/phys/DIMM4", "/phys/DIMM5",
          "/phys/DIMM6", "/phys/DIMM7", "/phys/DIMM8", "/phys/DIMM9",
          "/phys/DIMM10", "/phys/DIMM11", "/phys/DIMM12", "/phys/DIMM13",
          "/phys/DIMM14", "/phys/DIMM15", "/phys/DIMM16", "/phys/DIMM17",
          "/phys/DIMM18", "/phys/DIMM19", "/phys/DIMM20", "/phys/DIMM21",
          "/phys/DIMM22", "/phys/DIMM23", "/phys:connector:SYS_FAN0",
          "/phys:connector:SYS_FAN1", "/phys:connector:SYS_FAN2",
          "/phys:connector:SYS_FAN3", "/phys:connector:SYS_FAN4",
          "/phys:connector:SYS_FAN5", "/phys:connector:SYS_FAN6",
          "/phys:connector:SYS_FAN7", "/phys/SYS_FAN0", "/phys/SYS_FAN1",
          "/phys/SYS_FAN2", "/phys/SYS_FAN2/FAN2", "/phys/SYS_FAN2/FAN3",
          "/phys/SYS_FAN2/FAN4", "/phys/SYS_FAN2/FAN5", "/phys/SYS_FAN2/FAN6",
          "/phys/SYS_FAN2/FAN7", "/phys/SYS_FAN2:connector:FAN2",
          "/phys/SYS_FAN2:connector:FAN3", "/phys/SYS_FAN2:connector:FAN4",
          "/phys/SYS_FAN2:connector:FAN5", "/phys/SYS_FAN2:connector:FAN6",
          "/phys/SYS_FAN2:connector:FAN7", "/phys:connector:P48_PSU_L",
          "/phys/P48_PSU_L", "/phys:connector:TRAY", "/phys/TRAY",
          "/phys:connector:BIOS_SPI", "/phys/BIOS_SPI",
          "/phys:connector:CPU0_ANCHORS", "/phys/CPU0_ANCHORS",
          "/phys:connector:CPU1_ANCHORS", "/phys/CPU1_ANCHORS",
          "/phys:connector:NCSI", "/phys/NCSI"));
  EXPECT_THAT(
      backend_.GetPlugins(),
      UnorderedElementsAre(
          StringResult{"/phys", "indus"}, StringResult{"/phys/DIMM0", "ddr4"},
          StringResult{"/phys/DIMM1", "ddr4"},
          StringResult{"/phys/DIMM2", "ddr4"},
          StringResult{"/phys/DIMM3", "ddr4"},
          StringResult{"/phys/DIMM4", "ddr4"},
          StringResult{"/phys/DIMM5", "ddr4"},
          StringResult{"/phys/DIMM6", "ddr4"},
          StringResult{"/phys/DIMM7", "ddr4"},
          StringResult{"/phys/DIMM8", "ddr4"},
          StringResult{"/phys/DIMM9", "ddr4"},
          StringResult{"/phys/DIMM10", "ddr4"},
          StringResult{"/phys/DIMM11", "ddr4"},
          StringResult{"/phys/DIMM12", "ddr4"},
          StringResult{"/phys/DIMM13", "ddr4"},
          StringResult{"/phys/DIMM14", "ddr4"},
          StringResult{"/phys/DIMM15", "ddr4"},
          StringResult{"/phys/DIMM16", "ddr4"},
          StringResult{"/phys/DIMM17", "ddr4"},
          StringResult{"/phys/DIMM18", "ddr4"},
          StringResult{"/phys/DIMM19", "ddr4"},
          StringResult{"/phys/DIMM20", "ddr4"},
          StringResult{"/phys/DIMM21", "ddr4"},
          StringResult{"/phys/DIMM22", "ddr4"},
          StringResult{"/phys/DIMM23", "ddr4"},
          StringResult{"/phys/CPU0", "skylake"},
          StringResult{"/phys/CPU1", "skylake"},
          StringResult{"/phys/SYS_FAN0", "fan_40mm"},
          StringResult{"/phys/SYS_FAN1", "fan_40mm"},
          StringResult{"/phys/SYS_FAN2", "fan_assembly"},
          StringResult{"/phys/SYS_FAN2/FAN2", "fan_60mm"},
          StringResult{"/phys/SYS_FAN2/FAN3", "fan_60mm"},
          StringResult{"/phys/SYS_FAN2/FAN4", "fan_60mm"},
          StringResult{"/phys/SYS_FAN2/FAN5", "fan_60mm"},
          StringResult{"/phys/SYS_FAN2/FAN6", "fan_40mm"},
          StringResult{"/phys/SYS_FAN2/FAN7", "fan_40mm"},
          StringResult{"/phys/P48_PSU_L", "dc_rack_power_cable"},
          StringResult{"/phys/TRAY", "tray"},
          StringResult{"/phys/BIOS_SPI", "mx25l256"},
          StringResult{"/phys/CPU0_ANCHORS", "cooler"},
          StringResult{"/phys/CPU1_ANCHORS", "cooler"},
          StringResult{"/phys/NCSI", "hdmi_cable"}));
}

TEST_F(RedfishBackendTestWithMockup, GetCapacityInfo) {
  EXPECT_THAT(
      BackendQueryAllToArray<libredfish::PropertyCapacityMiB>(),
      UnorderedElementsAre(
          IntResult{"/phys/DIMM0", 32768}, IntResult{"/phys/DIMM1", 32768},
          IntResult{"/phys/DIMM2", 32768}, IntResult{"/phys/DIMM3", 32768},
          IntResult{"/phys/DIMM4", 32768}, IntResult{"/phys/DIMM5", 32768},
          IntResult{"/phys/DIMM6", 32768}, IntResult{"/phys/DIMM7", 32768},
          IntResult{"/phys/DIMM8", 32768}, IntResult{"/phys/DIMM9", 32768},
          IntResult{"/phys/DIMM10", 32768}, IntResult{"/phys/DIMM11", 32768},
          IntResult{"/phys/DIMM12", 32768}, IntResult{"/phys/DIMM13", 32768},
          IntResult{"/phys/DIMM14", 32768}, IntResult{"/phys/DIMM15", 32768},
          IntResult{"/phys/DIMM16", 32768}, IntResult{"/phys/DIMM17", 32768},
          IntResult{"/phys/DIMM18", 32768}, IntResult{"/phys/DIMM19", 32768},
          IntResult{"/phys/DIMM20", 32768}, IntResult{"/phys/DIMM21", 32768},
          IntResult{"/phys/DIMM22", 32768}, IntResult{"/phys/DIMM23", 32768}));
  EXPECT_THAT(
      BackendQueryAllToArray<libredfish::PropertyLogicalSizeMiB>(),
      UnorderedElementsAre(
          IntResult{"/phys/DIMM0", 32768}, IntResult{"/phys/DIMM1", 32768},
          IntResult{"/phys/DIMM2", 32768}, IntResult{"/phys/DIMM3", 32768},
          IntResult{"/phys/DIMM4", 32768}, IntResult{"/phys/DIMM5", 32768},
          IntResult{"/phys/DIMM6", 32768}, IntResult{"/phys/DIMM7", 32768},
          IntResult{"/phys/DIMM8", 32768}, IntResult{"/phys/DIMM9", 32768},
          IntResult{"/phys/DIMM10", 32768}, IntResult{"/phys/DIMM11", 32768},
          IntResult{"/phys/DIMM12", 32768}, IntResult{"/phys/DIMM13", 32768},
          IntResult{"/phys/DIMM14", 32768}, IntResult{"/phys/DIMM15", 32768},
          IntResult{"/phys/DIMM16", 32768}, IntResult{"/phys/DIMM17", 32768},
          IntResult{"/phys/DIMM18", 32768}, IntResult{"/phys/DIMM19", 32768},
          IntResult{"/phys/DIMM20", 32768}, IntResult{"/phys/DIMM21", 32768},
          IntResult{"/phys/DIMM22", 32768}, IntResult{"/phys/DIMM23", 32768}));
  EXPECT_THAT(
      BackendQueryAllToArray<libredfish::PropertyManufacturer>(),
      UnorderedElementsAre(StringResult{"/phys/DIMM0", "Kingston"},
                           StringResult{"/phys/DIMM1", "Kingston"},
                           StringResult{"/phys/DIMM2", "Kingston"},
                           StringResult{"/phys/DIMM3", "Kingston"},
                           StringResult{"/phys/DIMM4", "Kingston"},
                           StringResult{"/phys/DIMM5", "Kingston"},
                           StringResult{"/phys/DIMM6", "Kingston"},
                           StringResult{"/phys/DIMM7", "Kingston"},
                           StringResult{"/phys/DIMM8", "Kingston"},
                           StringResult{"/phys/DIMM9", "Kingston"},
                           StringResult{"/phys/DIMM10", "Kingston"},
                           StringResult{"/phys/DIMM11", "Kingston"},
                           StringResult{"/phys/DIMM12", "Kingston"},
                           StringResult{"/phys/DIMM13", "Kingston"},
                           StringResult{"/phys/DIMM14", "Kingston"},
                           StringResult{"/phys/DIMM15", "Kingston"},
                           StringResult{"/phys/DIMM16", "Kingston"},
                           StringResult{"/phys/DIMM17", "Kingston"},
                           StringResult{"/phys/DIMM18", "Kingston"},
                           StringResult{"/phys/DIMM19", "Kingston"},
                           StringResult{"/phys/DIMM20", "Kingston"},
                           StringResult{"/phys/DIMM21", "Kingston"},
                           StringResult{"/phys/DIMM22", "Kingston"},
                           StringResult{"/phys/DIMM23", "Kingston"},
                           StringResult{"/phys/CPU0", "Intel(R) Corporation"},
                           StringResult{"/phys/CPU1", "Intel(R) Corporation"}));
  EXPECT_THAT(BackendQueryAllToArray<libredfish::PropertyMemoryDeviceType>(),
              UnorderedElementsAre(StringResult{"/phys/DIMM0", "DDR4"},
                                   StringResult{"/phys/DIMM1", "DDR4"},
                                   StringResult{"/phys/DIMM2", "DDR4"},
                                   StringResult{"/phys/DIMM3", "DDR4"},
                                   StringResult{"/phys/DIMM4", "DDR4"},
                                   StringResult{"/phys/DIMM5", "DDR4"},
                                   StringResult{"/phys/DIMM6", "DDR4"},
                                   StringResult{"/phys/DIMM7", "DDR4"},
                                   StringResult{"/phys/DIMM8", "DDR4"},
                                   StringResult{"/phys/DIMM9", "DDR4"},
                                   StringResult{"/phys/DIMM10", "DDR4"},
                                   StringResult{"/phys/DIMM11", "DDR4"},
                                   StringResult{"/phys/DIMM12", "DDR4"},
                                   StringResult{"/phys/DIMM13", "DDR4"},
                                   StringResult{"/phys/DIMM14", "DDR4"},
                                   StringResult{"/phys/DIMM15", "DDR4"},
                                   StringResult{"/phys/DIMM16", "DDR4"},
                                   StringResult{"/phys/DIMM17", "DDR4"},
                                   StringResult{"/phys/DIMM18", "DDR4"},
                                   StringResult{"/phys/DIMM19", "DDR4"},
                                   StringResult{"/phys/DIMM20", "DDR4"},
                                   StringResult{"/phys/DIMM21", "DDR4"},
                                   StringResult{"/phys/DIMM22", "DDR4"},
                                   StringResult{"/phys/DIMM23", "DDR4"}));
  EXPECT_THAT(
      BackendQueryAllToArray<libredfish::PropertyOperatingSpeedMhz>(),
      UnorderedElementsAre(
          IntResult{"/phys/DIMM0", 2400}, IntResult{"/phys/DIMM1", 2400},
          IntResult{"/phys/DIMM2", 2400}, IntResult{"/phys/DIMM3", 2400},
          IntResult{"/phys/DIMM4", 2400}, IntResult{"/phys/DIMM5", 2400},
          IntResult{"/phys/DIMM6", 2400}, IntResult{"/phys/DIMM7", 2400},
          IntResult{"/phys/DIMM8", 2400}, IntResult{"/phys/DIMM9", 2400},
          IntResult{"/phys/DIMM10", 2400}, IntResult{"/phys/DIMM11", 2400},
          IntResult{"/phys/DIMM12", 2400}, IntResult{"/phys/DIMM13", 2400},
          IntResult{"/phys/DIMM14", 2400}, IntResult{"/phys/DIMM15", 2400},
          IntResult{"/phys/DIMM16", 2400}, IntResult{"/phys/DIMM17", 2400},
          IntResult{"/phys/DIMM18", 2400}, IntResult{"/phys/DIMM19", 2400},
          IntResult{"/phys/DIMM20", 2400}, IntResult{"/phys/DIMM21", 2400},
          IntResult{"/phys/DIMM22", 2400}, IntResult{"/phys/DIMM23", 2400}));
  EXPECT_THAT(
      BackendQueryAllToArray<libredfish::PropertyPartNumber>(),
      UnorderedElementsAre(StringResult{"/phys/DIMM0", "KF27000122-001"},
                           StringResult{"/phys/DIMM1", "KF27000122-001"},
                           StringResult{"/phys/DIMM2", "KF27000122-001"},
                           StringResult{"/phys/DIMM3", "KF27000122-001"},
                           StringResult{"/phys/DIMM4", "KF27000122-001"},
                           StringResult{"/phys/DIMM5", "KF27000122-001"},
                           StringResult{"/phys/DIMM6", "KF27000122-001"},
                           StringResult{"/phys/DIMM7", "KF27000122-001"},
                           StringResult{"/phys/DIMM8", "KF27000122-001"},
                           StringResult{"/phys/DIMM9", "KF27000122-001"},
                           StringResult{"/phys/DIMM10", "KF27000122-001"},
                           StringResult{"/phys/DIMM11", "KF27000122-001"},
                           StringResult{"/phys/DIMM12", "KF27000122-001"},
                           StringResult{"/phys/DIMM13", "KF27000122-001"},
                           StringResult{"/phys/DIMM14", "KF27000122-001"},
                           StringResult{"/phys/DIMM15", "KF27000122-001"},
                           StringResult{"/phys/DIMM16", "KF27000122-001"},
                           StringResult{"/phys/DIMM17", "KF27000122-001"},
                           StringResult{"/phys/DIMM18", "KF27000122-001"},
                           StringResult{"/phys/DIMM19", "KF27000122-001"},
                           StringResult{"/phys/DIMM20", "KF27000122-001"},
                           StringResult{"/phys/DIMM21", "KF27000122-001"},
                           StringResult{"/phys/DIMM22", "KF27000122-001"},
                           StringResult{"/phys/DIMM23", "KF27000122-001"},
                           StringResult{"/phys/CPU0", ""},
                           StringResult{"/phys/CPU1", ""}));
  EXPECT_THAT(BackendQueryAllToArray<libredfish::PropertySerialNumber>(),
              UnorderedElementsAre(StringResult{"/phys/DIMM0", "7032B000"},
                                   StringResult{"/phys/DIMM1", "7030B001"},
                                   StringResult{"/phys/DIMM2", "7030B002"},
                                   StringResult{"/phys/DIMM3", "7030B003"},
                                   StringResult{"/phys/DIMM4", "7030B004"},
                                   StringResult{"/phys/DIMM5", "7030B005"},
                                   StringResult{"/phys/DIMM6", "7030B006"},
                                   StringResult{"/phys/DIMM7", "7030B007"},
                                   StringResult{"/phys/DIMM8", "7030B008"},
                                   StringResult{"/phys/DIMM9", "7030B009"},
                                   StringResult{"/phys/DIMM10", "7030B010"},
                                   StringResult{"/phys/DIMM11", "7030B011"},
                                   StringResult{"/phys/DIMM12", "7030B012"},
                                   StringResult{"/phys/DIMM13", "7030B013"},
                                   StringResult{"/phys/DIMM14", "7030B014"},
                                   StringResult{"/phys/DIMM15", "7030B015"},
                                   StringResult{"/phys/DIMM16", "7030B016"},
                                   StringResult{"/phys/DIMM17", "7030B017"},
                                   StringResult{"/phys/DIMM18", "7030B018"},
                                   StringResult{"/phys/DIMM19", "7030B019"},
                                   StringResult{"/phys/DIMM20", "7030B020"},
                                   StringResult{"/phys/DIMM21", "7030B021"},
                                   StringResult{"/phys/DIMM22", "7030B022"},
                                   StringResult{"/phys/DIMM23", "7030B023"},
                                   StringResult{"/phys/CPU0", ""},
                                   StringResult{"/phys/CPU1", ""}));
}

TEST_F(RedfishBackendTestWithMockup, ProcessorProperties) {
  EXPECT_THAT(BackendQueryAllToArray<libredfish::PropertyTotalCores>(),
              UnorderedElementsAre(IntResult{"/phys/CPU0", 28},
                                   IntResult{"/phys/CPU1", 28}));
  EXPECT_THAT(BackendQueryAllToArray<libredfish::PropertyTotalEnabledCores>(),
              UnorderedElementsAre(IntResult{"/phys/CPU0", 28},
                                   IntResult{"/phys/CPU1", 28}));
  EXPECT_THAT(BackendQueryAllToArray<libredfish::PropertyTotalThreads>(),
              UnorderedElementsAre(IntResult{"/phys/CPU0", 56},
                                   IntResult{"/phys/CPU1", 56}));
  EXPECT_THAT(BackendQueryAllToArray<libredfish::PropertyMaxSpeedMhz>(),
              UnorderedElementsAre(IntResult{"/phys/CPU0", 4000},
                                   IntResult{"/phys/CPU1", 4000}));
}

TEST_F(RedfishBackendTestWithMockup, QueriesWork) {
  // Query for something which exists
  EXPECT_THAT(
      BackendQueryToArray<libredfish::PropertySerialNumber>({"/phys/DIMM0"}),
      UnorderedElementsAre(StringResult{"/phys/DIMM0", "7032B000"}));
  // Query for multiple things which exist
  EXPECT_THAT(BackendQueryToArray<libredfish::PropertySerialNumber>(
                  {"/phys/DIMM0", "/phys/DIMM23"}),
              UnorderedElementsAre(StringResult{"/phys/DIMM0", "7032B000"},
                                   StringResult{"/phys/DIMM23", "7030B023"}));
  // Query a devpath which does not exist
  EXPECT_THAT(
      BackendQueryToArray<libredfish::PropertySerialNumber>({"/phys/DIMM24"}),
      UnorderedElementsAre());
  // Query and include a devpath which doesn't exist
  EXPECT_THAT(BackendQueryToArray<libredfish::PropertySerialNumber>(
                  {"/phys/DIMM24", "/phys/DIMM23"}),
              UnorderedElementsAre(StringResult{"/phys/DIMM23", "7030B023"}));
  // Query only devpaths which don't exist
  EXPECT_THAT(BackendQueryToArray<libredfish::PropertySerialNumber>(
                  {"/phys/DIMM24", "/phys/DIMM25"}),
              UnorderedElementsAre());
}

}  // namespace
}  // namespace ecclesia
