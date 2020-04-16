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

// This header defines a function for merging graphs from several individual
// machine agents to produce unified set of devpaths.

// 1) Say there are n agents and each agent has a devpath
// graph generated already, then the MergeGraphs function can be used to
// merge all the subgraphs into a single unified graph. This helps the machine
// master to have a unified set of global devpaths. As a result of merging, we
// will also implicitly have the global devpath to local agent/devpath mapping.

// 2) This class handles two kinds of merging:
// a. A node from one agent plugs into a node from another agent.
// b. A node from one agent is the same as node from another agent.

// 3) There is also a prerequisite step that needs to be done (if applicable)
// a. Reverse the links in a graph
// This function does not handle this yet.

// 4)I assume that it will always be the root node of the appendant subgraph
// which will be merged with the the root node/any other node of the base
// subgraph. The above reversing the links in a graph step will help this
// assumption be true for non-straight forward cases.

#ifndef ECCLESIA_MMASTER_MIDDLES_DEVPATH_MERGER_H_
#define ECCLESIA_MMASTER_MIDDLES_DEVPATH_MERGER_H_

#include <string>

#include "absl/container/flat_hash_map.h"
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/devpath/graph_generator.h"

namespace ecclesia {
// This function merges several graphs.
// This function will take in a agent_graph map and the PluginGraphMergeSpec
// spec. This function will return true if the graph has been merged without
// any issues and false otherwise.
bool MergePluginGraphs(
    absl::flat_hash_map<std::string, DevpathGraph *> agent_graph_map,
    const PluginGraphMergeSpec &spec);

};  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MIDDLES_DEVPATH_MERGER_H_
