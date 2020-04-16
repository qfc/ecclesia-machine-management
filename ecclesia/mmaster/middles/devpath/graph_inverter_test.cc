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

#include "mmaster/middles/devpath/graph_inverter.h"

#include <algorithm>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "testing/base/public/gunit.h"
#include "absl/container/flat_hash_map.h"
#include "third_party/boost/allowed/graph/adjacency_list.hpp"
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/devpath/graph_generator.h"

namespace ecclesia {
namespace {

TEST(GraphTest, HasInvertedGraph) {
  std::vector<std::string> devpaths1 = {
      "/phys/B/C/D", "/phys/B/C", "/phys/F", "/phys/B", "/phys",
  };

  GraphGenerator generator1(devpaths1);
  DevpathGraph graph1;
  EXPECT_TRUE(generator1.GenerateGraph(&graph1, "a1"));

  PluginGraphMergeSpec spec;
  spec.set_root("a1");
  // Make node "/phys/B/C/D" as the new root in the agent "a1".
  auto* info1 = spec.add_invert_ops();
  info1->set_agent("a1");
  info1->set_new_root_devpath("/phys/B/C/D");
  // The set of connectors on the opposite end of B, C and D are W, X and Y
  // respectively
  info1->add_upstream_connectors("W");
  info1->add_upstream_connectors("X");
  info1->add_upstream_connectors("Y");

  absl::flat_hash_map<std::string, DevpathGraph*> agent_graph_map{
      {"a1", &graph1}};

  EXPECT_TRUE(InvertPluginGraphs(agent_graph_map, spec));
  std::vector<std::string> actual_plugins;
  std::vector<std::string> expected_plugins = {
      "/phys", "/phys/Y", "/phys/Y/X", "/phys/Y/X/W", "/phys/Y/X/W/F",
  };

  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(graph1); vi != vi_end; ++vi) {
    actual_plugins.push_back(graph1[*vi].global_devpath);
  }

  // Test if all the plugins have been created as vertices in the graph.
  std::sort(expected_plugins.begin(), expected_plugins.end());
  std::sort(actual_plugins.begin(), actual_plugins.end());
  EXPECT_EQ(expected_plugins, actual_plugins);
}

TEST(GraphTest, MissingUpstreamConnector) {
  // Same version of HasInvertedGraph test, but with an upstream connector
  // missing in invert_ops
  std::vector<std::string> devpaths1 = {
      "/phys/B/C/D", "/phys/B/C", "/phys/F", "/phys/B", "/phys",
  };

  GraphGenerator generator1(devpaths1);
  DevpathGraph graph1;
  EXPECT_TRUE(generator1.GenerateGraph(&graph1, "a1"));

  PluginGraphMergeSpec spec;
  spec.set_root("a1");
  // Make node "/phys/B/C/D" as the new root in the agent "a1".
  auto* info1 = spec.add_invert_ops();
  info1->set_agent("a1");
  info1->set_new_root_devpath("/phys/B/C/D");
  // The set of connectors on the opposite end of B, C and D are W, X and Y
  // respectively. Do not add Y to simulate failure condition.
  info1->add_upstream_connectors("W");
  info1->add_upstream_connectors("X");

  absl::flat_hash_map<std::string, DevpathGraph*> agent_graph_map{
      {"a1", &graph1}};
  EXPECT_FALSE(InvertPluginGraphs(agent_graph_map, spec));
}

TEST(GraphTest, HasInvertedGraphNoNewRootDevpath) {
  std::vector<std::string> devpaths1 = {
      "/phys/B/C/D", "/phys/B/C", "/phys/F", "/phys/B", "/phys",
  };

  GraphGenerator generator1(devpaths1);
  DevpathGraph graph1;
  EXPECT_TRUE(generator1.GenerateGraph(&graph1, "a1"));

  PluginGraphMergeSpec spec;
  spec.set_root("a1");
  // Make non existant node "/phys/B/C/K" as the new root in the agent "a1".
  auto* info1 = spec.add_invert_ops();
  info1->set_agent("a1");
  info1->set_new_root_devpath("/phys/B/C/K");
  info1->add_upstream_connectors("X");
  info1->add_upstream_connectors("Y");
  info1->add_upstream_connectors("Z");

  absl::flat_hash_map<std::string, DevpathGraph*> agent_graph_map{
      {"a1", &graph1}};

  EXPECT_FALSE(InvertPluginGraphs(agent_graph_map, spec));
}

TEST(GraphTest, HammurabiInvertedGraph) {
  std::vector<std::string> devpaths1 = {"/phys", "/phys/J2", "/phys/J7",
                                        "/phys/J7/DOWNLINK",
                                        "/phys/J7/DOWNLINK/J12"};
  GraphGenerator generator1(devpaths1);
  DevpathGraph graph1;
  EXPECT_TRUE(generator1.GenerateGraph(&graph1, "a1"));

  PluginGraphMergeSpec spec;
  spec.set_root("a1");
  // Make node "/phys/J7/DOWNLINK" the new root
  auto* info1 = spec.add_invert_ops();
  info1->set_agent("a1");
  info1->set_new_root_devpath("/phys/J7/DOWNLINK");
  info1->add_upstream_connectors("DOWNLINK");
  info1->add_upstream_connectors("J14");

  absl::flat_hash_map<std::string, DevpathGraph*> agent_graph_map{
      {"a1", &graph1}};

  EXPECT_TRUE(InvertPluginGraphs(agent_graph_map, spec));
  std::vector<std::string> actual_plugins;
  std::vector<std::string> expected_plugins = {
      "/phys",
      "/phys/J12",
      "/phys/J14",
      "/phys/J14/DOWNLINK",
      "/phys/J14/DOWNLINK/J2",
  };

  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(graph1); vi != vi_end; ++vi) {
    actual_plugins.push_back(graph1[*vi].global_devpath);
  }

  // Test if all the plugins have been created as vertices in the graph.
  std::sort(expected_plugins.begin(), expected_plugins.end());
  std::sort(actual_plugins.begin(), actual_plugins.end());
  EXPECT_EQ(expected_plugins, actual_plugins);
}

}  // namespace
}  // namespace ecclesia
