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

#include <iterator>
#include <optional>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "third_party/boost/allowed/graph/adjacency_list.hpp"
#include "third_party/boost/allowed/graph/copy.hpp"
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/devpath/graph_generator.h"

namespace ecclesia {
namespace {

// This function helps to merge two graphs from two agents where the node from
// one agent plugs into an node from another agent.
void PlugsIntoMerging(DevpathGraph *g1, DevpathVertex v_in_g1, DevpathGraph *g2,
                      DevpathVertex u_in_g2, const std::string &connector) {
  std::vector<DevpathVertex> orig2copy_data(boost::num_vertices(*g2));

  auto map = make_iterator_property_map(orig2copy_data.begin(),
                                        boost::get(boost::vertex_index, *g2));
  // Change the global devpaths for the second graph whose node is being plugged
  // into a node into the first graph.
  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(*g2); vi != vi_end; ++vi) {
    std::vector<absl::string_view> plugins =
        absl::StrSplit((*g2)[*vi].global_devpath, '/', absl::SkipEmpty());

    std::string temp_node =
        absl::StrJoin(plugins.begin() + 1, plugins.end(), "/");
    std::string node =
        absl::StrCat((*g1)[v_in_g1].global_devpath, "/", connector);
    if (!temp_node.empty()) {
      absl::StrAppend(&node, "/", temp_node);
    }
    (*g2)[*vi].global_devpath = node;
  }
  boost::copy_graph(*g2, *g1, boost::orig_to_copy(map));  // means g1 += g2

  DevpathVertex u_in_g1 = map[u_in_g2];

  // Add an edge between the node in second agent which plugs into a node from
  // first agent.
  boost::add_edge(v_in_g1, u_in_g1, *g1);
}

// This function helps to merge two graphs from two agents where the node from
// one agent is the same as a node from another agent.
void SameAsMerging(DevpathGraph *g1, DevpathVertex v_in_g1, DevpathGraph *g2,
                   DevpathVertex u_in_g2) {
  std::vector<DevpathVertex> orig2copy_data(boost::num_vertices(*g2));

  auto map = make_iterator_property_map(orig2copy_data.begin(),
                                        boost::get(boost::vertex_index, *g2));
  // Change the global devpaths for the second graph whose node is the same as
  // another node in the first graph.
  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(*g2); vi != vi_end; ++vi) {
    std::vector<absl::string_view> plugins =
        absl::StrSplit((*g2)[*vi].global_devpath, '/', absl::SkipEmpty());

    std::string temp_node =
        absl::StrJoin(plugins.begin() + 1, plugins.end(), "/");
    std::string node = (*g1)[v_in_g1].global_devpath;
    if (!temp_node.empty()) {
      absl::StrAppend(&node, "/", temp_node);
    }
    (*g2)[*vi].global_devpath = node;
  }
  boost::copy_graph(*g2, *g1, boost::orig_to_copy(map));  // means g1 += g2

  // Add edges between the node in first agent to all nodes which originate from
  // the node in the second agent(same as node from first agent).
  DevpathVertex u_in_g1 = map[u_in_g2];
  DevpathOutEdgeIt ei, ei_end;
  for (std::tie(ei, ei_end) = out_edges(u_in_g1, *g1); ei != ei_end; ++ei) {
    DevpathVertex dest = boost::target(*ei, *g1);
    boost::add_edge(v_in_g1, dest, *g1);
  }

  // Add the local devpaths from the nodes in second agent(same as node from
  // first agent) to the nodes in the first agent.
  if (!((*g1)[u_in_g1].local_agent_devpaths.empty())) {
    (*g1)[v_in_g1].local_agent_devpaths.insert(
        (*g1)[v_in_g1].local_agent_devpaths.end(),
        (*g1)[u_in_g1].local_agent_devpaths.begin(),
        (*g1)[u_in_g1].local_agent_devpaths.end());
  }
  // Remove the node in second agent which is same as node from first agent.
  boost::remove_vertex(u_in_g1, *g1);
}

// This function helps to get an order in which to process merging of graphs.
std::vector<PluginGraphMergeSpec::MergeOperation> GetValidNodeOrdering(
    const absl::flat_hash_map<std::string,
                              std::vector<PluginGraphMergeSpec::MergeOperation>>
        &agent_info_map,
    std::queue<std::string> *append_agent_list) {
  // Fetch the first non empty node ordering from the append_agent list.
  std::vector<PluginGraphMergeSpec::MergeOperation> ordering;
  while (!append_agent_list->empty()) {
    auto it = agent_info_map.find(append_agent_list->front());
    if (it != agent_info_map.end()) {
      ordering.assign(it->second.begin(), it->second.end());
      append_agent_list->pop();
      return ordering;
    }
    append_agent_list->pop();
  }
  return ordering;
}
}  // namespace

bool MergePluginGraphs(
    absl::flat_hash_map<std::string, DevpathGraph *> agent_graph_map,
    const PluginGraphMergeSpec &spec) {
  absl::flat_hash_map<std::string,
                      std::vector<PluginGraphMergeSpec::MergeOperation>>
      agent_info_map;
  if (spec.merge_ops().empty()) return true;
  // Store all the spec.graph_merger_info() in the agent_info map.
  for (const auto &info : spec.merge_ops()) {
    agent_info_map[info.base_agent()].push_back(info);
  }
  auto agent_it = agent_info_map.find(spec.root());
  if (agent_it == agent_info_map.end()) return false;
  std::vector<PluginGraphMergeSpec::MergeOperation> node_ordering =
      agent_it->second;

  std::queue<std::string> append_agent_list;
  // Repeat until we have agent graphs to process.
  while (!node_ordering.empty()) {
    for (const auto &info : node_ordering) {
      if (info.merge_case() ==
          PluginGraphMergeSpec_MergeOperation::MERGE_NOT_SET)
        continue;

      append_agent_list.push(info.appendant_agent());

      const std::string &source_agent = info.base_agent();
      std::string source_devpath;
      if (info.merge_case() == PluginGraphMergeSpec_MergeOperation::kSameNode) {
        source_devpath = info.same_node().base_devpath();
      } else {
        source_devpath = info.plugged_in_node().base_devpath();
      }
      const std::string &destination_agent = info.appendant_agent();
      std::string destination_devpath;
      if (info.merge_case() == PluginGraphMergeSpec_MergeOperation::kSameNode) {
        destination_devpath = info.same_node().appendant_devpath();
      } else {
        destination_devpath = info.plugged_in_node().appendant_devpath();
      }

      DevpathGraph *source_graph = agent_graph_map[spec.root()];
      absl::optional<DevpathVertex> source_vertex;

      const auto source_vertex_it =
          FindVertex(*source_graph, source_agent, source_devpath);
      const auto end_vertex_it = boost::vertices(*source_graph).second;

      if (source_vertex_it != end_vertex_it) {
        source_vertex = *source_vertex_it;
      }

      DevpathGraph *destination_graph = agent_graph_map[destination_agent];
      absl::optional<DevpathVertex> destination_vertex;

      const auto destination_vertex_it = FindVertex(
          *destination_graph, destination_agent, destination_devpath);
      const auto destination_end_vertex_it =
          boost::vertices(*destination_graph).second;

      if (destination_vertex_it != destination_end_vertex_it) {
        destination_vertex = *destination_vertex_it;
      }

      if (info.merge_case() == PluginGraphMergeSpec_MergeOperation::kSameNode) {
        // Process same as type graph merging.
        if (source_vertex.has_value() && destination_vertex.has_value()) {
          SameAsMerging(source_graph, source_vertex.value(), destination_graph,
                        destination_vertex.value());
        } else {
          return false;
        }
      } else if (info.merge_case() ==
                 PluginGraphMergeSpec_MergeOperation::kPluggedInNode) {
        // Process plugs into type graph merging.
        std::string connector = info.plugged_in_node().connector();
        if (source_vertex.has_value() && destination_vertex.has_value()) {
          PlugsIntoMerging(source_graph, source_vertex.value(),
                           destination_graph, destination_vertex.value(),
                           connector);
        } else {
          return false;
        }
      }
    }
    node_ordering.clear();
    node_ordering = GetValidNodeOrdering(agent_info_map, &append_agent_list);
  }

  // Check if the merged final graph has a cycle because we expect an acyclic
  // graph.
  if (HasCycle(*(agent_graph_map[spec.root()]))) {
    return false;
  }
  return true;
}
}  // namespace ecclesia
