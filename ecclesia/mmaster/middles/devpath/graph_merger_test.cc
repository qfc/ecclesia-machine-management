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

#include "mmaster/middles/devpath/graph_merger.h"

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

TEST(GraphTest, SingleGraphNoMerging) {
  std::vector<std::string> devpaths1 = {"/phys/B", "/phys"};
  GraphGenerator generator1(devpaths1);
  DevpathGraph graph1;
  EXPECT_TRUE(generator1.GenerateGraph(&graph1, "a1"));

  absl::flat_hash_map<std::string, DevpathGraph*> agent_graph_map{
      {"a1", &graph1}};

  PluginGraphMergeSpec spec;
  spec.set_root("a1");
  EXPECT_TRUE(MergePluginGraphs(agent_graph_map, spec));

  std::vector<std::string> actual_plugins;
  std::vector<std::string> expected_plugins = {
      "/phys",
      "/phys/B",
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

TEST(GraphTest, HasMergedGraph) {
  std::vector<std::string> devpaths1 = {"/phys/B", "/phys"};
  GraphGenerator generator1(devpaths1);
  DevpathGraph graph1;
  EXPECT_TRUE(generator1.GenerateGraph(&graph1, "a1"));

  std::vector<std::string> devpaths2 = {"/phys/D", "/phys"};
  GraphGenerator generator2(devpaths2);
  DevpathGraph graph2;
  EXPECT_TRUE(generator2.GenerateGraph(&graph2, "a2"));

  std::vector<std::string> devpaths3 = {"/phys/F", "/phys"};
  GraphGenerator generator3(devpaths3);
  DevpathGraph graph3;
  EXPECT_TRUE(generator3.GenerateGraph(&graph3, "a3"));

  std::vector<std::string> devpaths4 = {"/phys/H", "/phys"};
  GraphGenerator generator4(devpaths4);
  DevpathGraph graph4;
  EXPECT_TRUE(generator4.GenerateGraph(&graph4, "a4"));

  PluginGraphMergeSpec spec;
  spec.set_root("a1");
  // Agent "a2" has a node which plugs into a node in "a1"
  auto* info1 = spec.add_merge_ops();
  info1->set_base_agent("a1");
  info1->set_appendant_agent("a2");
  auto* plugged_in_node = info1->mutable_plugged_in_node();
  plugged_in_node->set_connector("PADS1");
  plugged_in_node->set_base_devpath("/phys/B");
  plugged_in_node->set_appendant_devpath("/phys");

  // Agent "a3" has a node which is same as a node in "a1"
  auto* info2 = spec.add_merge_ops();
  info2->set_base_agent("a1");
  info2->set_appendant_agent("a3");
  auto* same_node = info2->mutable_same_node();
  same_node->set_base_devpath("/phys/B");
  same_node->set_appendant_devpath("/phys");

  // Agent "a4" has a node which plugs into a node in "a2"
  auto* info3 = spec.add_merge_ops();
  info3->set_base_agent("a2");
  info3->set_appendant_agent("a4");
  auto* plugged_in_node2 = info3->mutable_plugged_in_node();
  plugged_in_node2->set_connector("PADS2");
  plugged_in_node2->set_base_devpath("/phys");
  plugged_in_node2->set_appendant_devpath("/phys/H");
  absl::flat_hash_map<std::string, DevpathGraph *> agent_graph_map{
      {"a1", &graph1}, {"a2", &graph2}, {"a3", &graph3}, {"a4", &graph4}};

  EXPECT_TRUE(MergePluginGraphs(agent_graph_map, spec));

  std::vector<std::string> actual_plugins;
  std::vector<std::string> expected_plugins = {
      "/phys",
      "/phys/B",
      "/phys/B/PADS1",
      "/phys/B/PADS1/D",
      "/phys/B/F",
      "/phys/B/PADS1/PADS2",
      "/phys/B/PADS1/PADS2/H",
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

TEST(GraphTest, SameAsMergingNoBasedDevpath) {
  std::vector<std::string> devpaths1 = {"/phys/B", "/phys"};
  GraphGenerator generator1(devpaths1);
  DevpathGraph graph1;
  EXPECT_TRUE(generator1.GenerateGraph(&graph1, "a1"));

  std::vector<std::string> devpaths3 = {"/phys/F", "/phys"};
  GraphGenerator generator3(devpaths3);
  DevpathGraph graph3;
  EXPECT_TRUE(generator3.GenerateGraph(&graph3, "a3"));

  PluginGraphMergeSpec spec;
  spec.set_root("a1");
  // Agent "a3" has a node which is same as a node in "a1"
  auto* info2 = spec.add_merge_ops();
  info2->set_base_agent("a1");
  info2->set_appendant_agent("a3");
  auto* same_node = info2->mutable_same_node();
  same_node->set_base_devpath("/phys/K");
  same_node->set_appendant_devpath("/phys");

  absl::flat_hash_map<std::string, DevpathGraph*> agent_graph_map{
      {"a1", &graph1}, {"a3", &graph3}};

  EXPECT_FALSE(MergePluginGraphs(agent_graph_map, spec));
}
}  // namespace ecclesia
