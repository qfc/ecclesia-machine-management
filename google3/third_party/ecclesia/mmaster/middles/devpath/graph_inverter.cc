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
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/devpath/graph_generator.h"

namespace ecclesia {

bool InvertPluginGraphs(
    const absl::flat_hash_map<std::string, DevpathGraph *> &agent_graph_map,
    const PluginGraphMergeSpec &spec) {
  for (const auto &info : spec.invert_ops()) {
    const std::string &agent = info.agent();
    const std::string &root_devpath = info.new_root_devpath();

    auto agent_it = agent_graph_map.find(agent);
    if (agent_it == agent_graph_map.end()) return false;
    DevpathGraph *graph = agent_it->second;

    // Invert the edges as per the configuration.
    // At first, find the vertex which is the old root.
    // Then work through the path from the old root to the new root and
    // invert all edges in between. Leave other paths untouched.
    std::vector<absl::string_view> plugins =
        absl::StrSplit(root_devpath, '/', absl::SkipEmpty());

    // If there are N plugins, there are N-1 upstream connectors in between
    if (plugins.size() != (info.upstream_connectors_size() + 1)) {
      return false;
    }

    std::vector<absl::string_view> nodes;
    DevpathVertex curr_vertex;
    absl::optional<DevpathVertex> previous_vertex;
    int index = 0;
    for (absl::string_view plugin : plugins) {
      nodes.push_back(plugin);
      std::string node =
          absl::StrCat("/", absl::StrJoin(nodes.begin(), nodes.end(), "/"));
      const auto vertex_it = FindVertex(*graph, agent, absl::StrCat(node));
      const auto end_vertex_it = boost::vertices(*graph).second;
      if (vertex_it == end_vertex_it) {
        return false;
      }

      // Temporarily update the global_devpaths of the nodes between the old
      // root and new root with upstream_connectors from invert_ops spec. This
      // will be required when re-generating the correct devpaths later.
      // Change the devpath of the new root to be /phys
      if (index < info.upstream_connectors_size()) {
        (*graph)[*vertex_it].global_devpath =
            absl::StrCat("/", info.upstream_connectors(index++));
      } else {
        (*graph)[*vertex_it].global_devpath = "/phys";
      }
      if (!previous_vertex.has_value()) {
        previous_vertex = *vertex_it;
        continue;
      }
      curr_vertex = *vertex_it;
      boost::add_edge(curr_vertex, previous_vertex.value(), *graph);
      boost::remove_edge(previous_vertex.value(), curr_vertex, *graph);
      previous_vertex = curr_vertex;
    }

    // Give correct values to the global devpaths in all the vertices.
    // For all vertices in the graph change the global devpath to hold only the
    // particular plugin name. Say if a global devpath of a vertex is "/phys/B",
    // then change it to become "/B".
    DevpathVertexIt vi, vi_end;
    for (std::tie(vi, vi_end) = vertices(*graph); vi != vi_end; ++vi) {
      auto &global_devpath = (*graph)[*vi].global_devpath;
      std::vector<absl::string_view> plugins =
          absl::StrSplit(global_devpath, '/', absl::SkipEmpty());
      global_devpath = absl::StrCat("/", plugins.back());
    }
    std::queue<DevpathVertex> vertex_queue;
    vertex_queue.push(curr_vertex);

    // Start with the new root vertex. For every outgoing edge, get the target
    // vertex and change the global devpath by prefixing the global devpath
    // of the source vertex. Do this for all vertices so that all paths are
    // covered.
    DevpathOutEdgeIt ei, ei_end;
    while (!vertex_queue.empty()) {
      DevpathVertex vertex = vertex_queue.front();
      vertex_queue.pop();
      for (std::tie(ei, ei_end) = out_edges(vertex, *graph); ei != ei_end;
           ++ei) {
        DevpathVertex target = boost::target((*ei), *graph);
        DevpathVertex source = boost::source((*ei), *graph);
        (*graph)[target].global_devpath = absl::StrCat(
            (*graph)[source].global_devpath, (*graph)[target].global_devpath);
        vertex_queue.push(target);
      }
    }
  }
  return true;
}

};  // namespace ecclesia
