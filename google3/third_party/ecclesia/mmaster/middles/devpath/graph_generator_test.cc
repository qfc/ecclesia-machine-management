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

#include "mmaster/middles/devpath/graph_generator.h"

#include <algorithm>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "testing/base/public/gunit.h"
#include "third_party/boost/allowed/graph/adjacency_list.hpp"

namespace ecclesia {

TEST(GraphTest, HasGeneratedGraph) {
  std::vector<std::string> devpaths = {
      "/phys/CPU0",
      "/phys/CPU1",
      "/phys/SYS_FAN0",
      "/phys/SYS_FAN1",
      "/phys/SYS_FAN2",
      "/phys/SYS_FAN2/FAN2",
      "/phys/SYS_FAN2/FAN3",
      "/phys/SYS_FAN2/FAN4",
      "/phys/SYS_FAN2/FAN5",
      "/phys/SYS_FAN2/FAN6",
      "/phys/SYS_FAN2/FAN7",
      "/phys/P48_PSU_L",
      "/phys/TRAY",
      "/phys/BIOS_SPI",
      "/phys/CPU0_ANCHORS",
      "/phys/CPU1_ANCHORS",
      "/phys/NCSI",
      "/phys/PE1",
      "/phys/PE1/NET0",
      "/phys/KA0",
      "/phys/KA2",
      "/phys/DIMM0",
      "/phys/DIMM2",
      "/phys/DIMM4",
      "/phys/DIMM7",
      "/phys/DIMM9",
      "/phys/DIMM11",
      "/phys/DIMM12",
      "/phys/DIMM14",
      "/phys/DIMM16",
      "/phys/DIMM19",
      "/phys/DIMM21",
      "/phys",
      "/phys/DIMM23",
      "/phys/SATA0",
      "/phys/SATA0/DOWNLINK",
      "/phys/PE2",
      "/phys/PE2/CDFP",
      "/phys/PE2/CDFP/DOWNLINK",
      "/phys/PE2/CDFP/DOWNLINK/PE",
      "/phys/PE2/CDFP/DOWNLINK/PE/P12V_PCIE",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/FAN_B/DOWNLINK",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/FAN_B/DOWNLINK/FAN0",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/FAN_B/DOWNLINK/FAN1",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/FAN_B/DOWNLINK/FAN2",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/FAN_B",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/P12V_SYS",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/U2_0",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/BMC",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/BMC/FLASH",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/U2_2",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/U2_3",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/U2_4",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/U2_5",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/U2_6",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_0/DOWNLINK/U2_7",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_1",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_2",
      "/phys/PE2/CDFP/DOWNLINK/PE/SlimSAS_3",
      "/phys/PE3",
      "/phys/PE3/CDFP",
      "/phys/PE3/CDFP/DOWNLINK",
  };

  GraphGenerator generator(devpaths);
  DevpathGraph graph;
  EXPECT_TRUE(generator.GenerateGraph(&graph, "agent1"));

  std::vector<std::string> actual_plugins;

  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; ++vi) {
    actual_plugins.push_back(graph[*vi].global_devpath);
  }

  // Test if all the plugins have been created as vertices in the graph.
  std::sort(devpaths.begin(), devpaths.end());
  std::sort(actual_plugins.begin(), actual_plugins.end());
  EXPECT_EQ(devpaths, actual_plugins);
}

TEST(GraphTest, HasGeneratedGraphNoBarePaths) {
  std::vector<std::string> devpaths = {
      "/MOBO/CPU0",
      "/MOBO/CPU1",
      "/MOBO",
  };

  GraphGenerator generator(devpaths);
  DevpathGraph graph;
  EXPECT_FALSE(generator.GenerateGraph(&graph, "agent1"));
}
}  // namespace ecclesia
