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

// This header defines a function for inverting a graph.

// 2) This class handles one kind of inverting:
// a. Make a new node node as the root in a graph. For new node to become root,
//    we may need to invert some edges and change the global devpaths assigned
//    to the vertices.

#ifndef ECCLESIA_MMASTER_MIDDLES_DEVPATH_GRAPH_INVERTER_H_
#define ECCLESIA_MMASTER_MIDDLES_DEVPATH_GRAPH_INVERTER_H_

#include <string>

#include "absl/container/flat_hash_map.h"
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/devpath/graph_generator.h"

namespace ecclesia {

// This function will invert a graph based on the configuration.
// This function will take in a agent_graph map and the PluginGraphMergeSpec
// spec. This function will return true if the graph has been inverted without
// any issues and false otherwise.
bool InvertPluginGraphs(
    const absl::flat_hash_map<std::string, DevpathGraph *> &agent_graph_map,
    const PluginGraphMergeSpec &spec);

}  // namespace ecclesia
#endif  // ECCLESIA_MMASTER_MIDDLES_DEVPATH_GRAPH_INVERTER_H_
