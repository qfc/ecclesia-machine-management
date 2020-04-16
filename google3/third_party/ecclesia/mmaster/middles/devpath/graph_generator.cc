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
#include <iterator>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "third_party/boost/allowed/graph/adjacency_list.hpp"
#include "lib/devpath/devpath.h"

namespace ecclesia {

// This function is used to find if the vertex already exists in the graph.
DevpathVertexIt FindVertex(const DevpathGraph &g, absl::string_view agent,
                           absl::string_view devpath) {
  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(g); vi != vi_end; ++vi) {
    for (const auto &agent_devpath : (g[*vi].local_agent_devpaths)) {
      if ((agent_devpath.local_devpath == devpath) &&
          (agent_devpath.local_agent == agent))
        return vi;
    }
  }
  return vi_end;
}

// This function determines if the input graph has a cycle.
bool HasCycle(const DevpathGraph &graph) {
  // Cycle detection
  // TODO(jaghu) : Implement logic to detect cycle detection.
  return false;
}

GraphGenerator::GraphGenerator(absl::Span<const std::string> plugins) {
  regular_devpaths_.reserve(plugins.size());
  std::copy(plugins.begin(), plugins.end(),
            std::back_inserter(regular_devpaths_));
  std::sort(regular_devpaths_.begin(), regular_devpaths_.end());
}

bool GraphGenerator::GenerateGraph(DevpathGraph *graph,
                                   const std::string &agent) {
  // Generate a base graph which represents all the plugins and their plugged in
  // connectors.
  for (const std::string &devpath : regular_devpaths_) {
    if (!(IsValidDevpath(devpath))) return false;
    std::vector<absl::string_view> plugins =
        absl::StrSplit(devpath, '/', absl::SkipEmpty());
    // Skip if we don't have bare paths.
    // The root vertex will come first because the vector of devpaths has been
    // sorted.
    if (plugins.size() == 1) {
      std::vector<LocalAgentDevpath> local_agent_devpath = {
          {absl::StrCat("/", plugins[0]), agent}};
      const DevpathVertexProperties vertex_property{
          absl::StrCat("/", plugins[0]), local_agent_devpath};
      boost::add_vertex(vertex_property, *graph);
      continue;
    }
    std::vector<absl::string_view> nodes;
    DevpathVertex previous_vertex;
    for (absl::string_view plugin : plugins) {
      nodes.push_back(plugin);
      std::string node =
          absl::StrCat("/", absl::StrJoin(nodes.begin(), nodes.end(), "/"));
      // Find if the vertex already exists.
      const auto vertex_it = FindVertex(*graph, agent, absl::StrCat(node));
      const auto end_vertex_it = boost::vertices(*graph).second;

      // If the vertex exists, then make that the previous vertex.
      // If the vextex does not exist, then create a new vertex and create a
      // edge from the previous vertex to the current vertex and the current
      // vertex as the previous vertex.
      if (vertex_it != end_vertex_it) {
        previous_vertex = *vertex_it;
        continue;
      } else {
        std::vector<LocalAgentDevpath> local_agent_devpath = {{node, agent}};
        const DevpathVertexProperties vertex_property{node,
                                                      local_agent_devpath};
        DevpathVertex curr_vertex = boost::add_vertex(vertex_property, *graph);
        boost::add_edge(previous_vertex, curr_vertex, *graph);
        previous_vertex = curr_vertex;
      }
    }
  }

  // Check if the generated graph has a cycle because we expect an acyclic
  // graph.
  if (HasCycle(*graph)) {
    return false;
  }
  return true;
}
}  // namespace ecclesia
