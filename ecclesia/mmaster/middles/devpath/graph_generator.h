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

// This header defines a class for generating a graph for an individual machine
// agent. The machine agent will need to supply its own local devpaths and then
// this class will help to generate the graph.

#ifndef PLATFORMS_ECCLESIA_MMASTER_GRAPH_LIB_GRAPH_H_
#define PLATFORMS_ECCLESIA_MMASTER_GRAPH_LIB_GRAPH_H_

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "third_party/boost/allowed/graph/adjacency_list.hpp"

namespace ecclesia {

// This struct documents the local agent name and a devpath as seen by the
// agent.
struct LocalAgentDevpath {
  std::string local_devpath;
  std::string local_agent;
};

// This struct documents the vertex properties.
struct DevpathVertexProperties {
  explicit DevpathVertexProperties() {}
  DevpathVertexProperties(
      std::string local_devpath,
      std::vector<LocalAgentDevpath> local_agent_devpath_vec)
      : global_devpath(std::move(local_devpath)) {
    local_agent_devpaths.assign(local_agent_devpath_vec.begin(),
                                local_agent_devpath_vec.end());
  }

  // Represents a global plugin devpath as viewed from the machine master
  std::string global_devpath;
  // Represents local plugin devpaths as viewed from the machine agent
  std::vector<LocalAgentDevpath> local_agent_devpaths;
  // Represents the unplugged connectors that the plugin may contain
  std::vector<std::string> unplugged_connectors;
  // Represents the contained devices that the plugin may contain
  std::vector<std::string> contained_devices;
};

// Used to represent a graph with vertices as plugins and egdes as connectors.
using DevpathGraph =
    boost::adjacency_list<boost::setS,             // OutEdgeList is a set
                          boost::vecS,             // VertexList is a vector
                          boost::directedS,        // Edges are directed
                          DevpathVertexProperties  // Vertex properties
                          >;
// Used to represent a vertex in the graph.
using DevpathVertex = boost::graph_traits<DevpathGraph>::vertex_descriptor;
// Used to represent a iteratror which can iterate thorugh all the vertices in
// the graph.
using DevpathVertexIt = boost::graph_traits<DevpathGraph>::vertex_iterator;
// Used to represent a iteratror which can iterate thorugh all the edges coming
// out from a vertex in the graph.
using DevpathOutEdgeIt = boost::graph_traits<DevpathGraph>::out_edge_iterator;

// Each vertex in the graph will represent a plugin and will contain the devpath
// for the plugin and may optionally contain any unplugged connectors and
// contained devices as available in the plugin.
// Each edge in the graph will represent a connector connecting two plugins
// and will be directed from one vertex to another.

// This class expects the local plugin devpaths to have been correctly generated
// using the bare path generation algorithm. The graph generated will be a DAG
// because of the nature of bare paths.
class GraphGenerator {
 public:
  explicit GraphGenerator(absl::Span<const std::string> plugins);

  GraphGenerator(const GraphGenerator &other) = delete;
  GraphGenerator &operator=(const GraphGenerator &other) = delete;

  // Function generates a graph.
  // This function will take in a empty graph and fill in the all the required
  // vertices and edges for the graph.
  // This function will return true if the graph has been generated without any
  // issues and false otherwise.
  bool GenerateGraph(DevpathGraph *g, const std::string &agent);

 private:
  // The implementation of this class expects all the vectors to be sorted,
  // which is being done in the constructor of the class.
  std::vector<std::string> regular_devpaths_;
};

// This function is used to find if the vertex already exists in the graph.
DevpathVertexIt FindVertex(const DevpathGraph &g, absl::string_view agent,
                           absl::string_view devpath);

// This function checks if the input graph has a cycle.
bool HasCycle(const DevpathGraph &graph);

}  // namespace ecclesia

#endif  // PLATFORMS_ECCLESIA_MMASTER_GRAPH_LIB_GRAPH_H_
