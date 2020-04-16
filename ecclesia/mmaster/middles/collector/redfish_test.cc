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

#include "mmaster/middles/collector/redfish.h"

#include <utility>
#include <vector>

#include "net/proto2/contrib/parse_proto/parse_text_proto.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "lib/redfish/test_mockup.h"
#include "mmaster/backends/redfish/redfish.h"
#include "mmaster/config/config.proto.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {
namespace {

using ::testing::EqualsProto;

TEST(RedfishResourceCollectorTest, EnumerateMockup) {
  libredfish::TestingMockupServer mockup(
      "google3/redfish_mockups/indus_hmb_cn_mockup.sar");
  auto mockup_config = mockup.GetConfig();
  AgentConfig config;
  config.set_name("redfish");
  config.mutable_redfish()->set_hostname(mockup_config.hostname);
  config.mutable_redfish()->set_port(mockup_config.port);

  RedfishBackend backend(config);
  RedfishResourceCollector collector(&backend);

  std::vector<EnumerateAssemblyResponse> responses;
  EnumerateAssemblyResponse resp;
  collector.Enumerate(&resp, [&]() { responses.push_back(std::move(resp)); });
  ASSERT_THAT(responses,
              ::testing::UnorderedElementsAre(
                  EqualsProto("id { devpath: '/phys' }"),
                  EqualsProto("id { devpath: '/phys/DIMM0' }"),
                  EqualsProto("id { devpath: '/phys/DIMM1' }"),
                  EqualsProto("id { devpath: '/phys/DIMM2' }"),
                  EqualsProto("id { devpath: '/phys/DIMM3' }"),
                  EqualsProto("id { devpath: '/phys/DIMM4' }"),
                  EqualsProto("id { devpath: '/phys/DIMM5' }"),
                  EqualsProto("id { devpath: '/phys/DIMM6' }"),
                  EqualsProto("id { devpath: '/phys/DIMM7' }"),
                  EqualsProto("id { devpath: '/phys/DIMM8' }"),
                  EqualsProto("id { devpath: '/phys/DIMM9' }"),
                  EqualsProto("id { devpath: '/phys/DIMM10' }"),
                  EqualsProto("id { devpath: '/phys/DIMM11' }"),
                  EqualsProto("id { devpath: '/phys/DIMM12' }"),
                  EqualsProto("id { devpath: '/phys/DIMM13' }"),
                  EqualsProto("id { devpath: '/phys/DIMM14' }"),
                  EqualsProto("id { devpath: '/phys/DIMM15' }"),
                  EqualsProto("id { devpath: '/phys/DIMM16' }"),
                  EqualsProto("id { devpath: '/phys/DIMM17' }"),
                  EqualsProto("id { devpath: '/phys/DIMM18' }"),
                  EqualsProto("id { devpath: '/phys/DIMM19' }"),
                  EqualsProto("id { devpath: '/phys/DIMM20' }"),
                  EqualsProto("id { devpath: '/phys/DIMM21' }"),
                  EqualsProto("id { devpath: '/phys/DIMM22' }"),
                  EqualsProto("id { devpath: '/phys/DIMM23' }"),
                  EqualsProto("id { devpath: '/phys/CPU0' }"),
                  EqualsProto("id { devpath: '/phys/CPU1' }"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN0'}"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN1'}"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN2'}"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN2/FAN2'}"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN2/FAN3'}"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN2/FAN4'}"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN2/FAN5'}"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN2/FAN6'}"),
                  EqualsProto("id { devpath: '/phys/SYS_FAN2/FAN7'}"),
                  EqualsProto("id { devpath: '/phys/P48_PSU_L' }"),
                  EqualsProto("id { devpath: '/phys/TRAY' }"),
                  EqualsProto("id { devpath: '/phys/BIOS_SPI' }"),
                  EqualsProto("id { devpath: '/phys/CPU0_ANCHORS' }"),
                  EqualsProto("id { devpath: '/phys/CPU1_ANCHORS' }"),
                  EqualsProto("id { devpath: '/phys/NCSI' }")));
}

TEST(RedfishResourceCollectorTest, QueryMockup) {
  libredfish::TestingMockupServer mockup(
      "google3/redfish_mockups/indus_hmb_cn_mockup.sar");
  auto mockup_config = mockup.GetConfig();
  AgentConfig config;
  config.set_name("redfish");
  config.mutable_redfish()->set_hostname(mockup_config.hostname);
  config.mutable_redfish()->set_port(mockup_config.port);

  RedfishBackend backend(config);
  RedfishResourceCollector collector(&backend);

  {
    AssemblyIdentifier id = PARSE_TEXT_PROTO("devpath: '/phys'");
    QueryAssemblyResponse response =
        collector.Query(id, PARSE_TEXT_PROTO(R"pb(paths: 'name')pb"));
    EXPECT_THAT(response, EqualsProto(
                              R"pb(
                                status { code: 0 }
                                name: 'indus'
                              )pb"));
  }

  {
    AssemblyIdentifier id = PARSE_TEXT_PROTO("devpath: '/phys/CPU0'");
    QueryAssemblyResponse response =
        collector.Query(id, PARSE_TEXT_PROTO(R"pb(paths: 'name')pb"));
    EXPECT_THAT(response, EqualsProto(
                              R"pb(
                                status { code: 0 }
                                name: 'skylake'
                              )pb"));
  }

  {
    AssemblyIdentifier id = PARSE_TEXT_PROTO("devpath: '/phys/DIMM12'");
    QueryAssemblyResponse response =
        collector.Query(id, PARSE_TEXT_PROTO(R"pb(paths: 'name')pb"));
    EXPECT_THAT(response, EqualsProto(
                              R"pb(
                                status { code: 0 }
                                name: 'ddr4'
                              )pb"));
  }
}

TEST(RedfishResourceCollectorTest, QueryMockupNotFound) {
  libredfish::TestingMockupServer mockup(
      "google3/redfish_mockups/indus_hmb_cn_mockup.sar");
  auto mockup_config = mockup.GetConfig();
  AgentConfig config;
  config.set_name("redfish");
  config.mutable_redfish()->set_hostname(mockup_config.hostname);
  config.mutable_redfish()->set_port(mockup_config.port);

  RedfishBackend backend(config);
  RedfishResourceCollector collector(&backend);

  {
    AssemblyIdentifier id = PARSE_TEXT_PROTO("devpath: '/phys/bad'");
    QueryAssemblyResponse response =
        collector.Query(id, PARSE_TEXT_PROTO(R"pb(paths: 'name')pb"));
    EXPECT_THAT(response, EqualsProto(
                              R"pb(
                                status { code: 5 }
                              )pb"));
  }
}

}  // namespace
}  // namespace ecclesia
