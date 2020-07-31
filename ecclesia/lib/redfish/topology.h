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

#ifndef ECCLESIA_LIB_REDFISH_TOPOLOGY_H_
#define ECCLESIA_LIB_REDFISH_TOPOLOGY_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/types/optional.h"
#include "ecclesia/lib/redfish/interface.h"
#include "ecclesia/lib/redfish/types.h"

namespace libredfish {

// Node represents a single devpath from the Redfish backend and its associated
// system data. There is a one-to-one mapping between Redfish Assembly
// Components and Nodes.
struct Node {
  // Name property value from the Redfish Component representing this Node.
  std::string name;
  // Local devpath assigned to this Node.
  std::string local_devpath;
  // Node type represents the physical type of this Node.
  NodeType type;
  // Associated URIs that provide logical system information to this Node.
  std::vector<std::string> associated_uris;
};

// NodeTopology represents the collection of Nodes comprising a Redfish backend.
// Additional data structures are provided in order to conveniently look up
// specific nodes.
struct NodeTopology {
  // All nodes fround from the redfish backend.
  std::vector<std::unique_ptr<Node>> nodes;

  // Map of a URI to all associated Nodes. If the URI is fetched, these Nodes
  // can have their properties updated from that information.
  absl::flat_hash_map<std::string, std::vector<Node *>>
      uri_to_associated_node_map;
  // Map of a domain devpath to a Node.
  absl::flat_hash_map<std::string, Node *> devpath_to_node_map;
};

NodeTopology CreateTopologyFromRedfish(RedfishInterface *redfish_intf);

// Returns true if both provided NodeTopologies have the same nodes. Nodes are
// matched by their name, local_devpath, and type fields only. This does not
// detect changes in the internal maps to nodes.
bool NodeTopologiesHaveTheSameNodes(const NodeTopology &n1,
                                    const NodeTopology &n2);

}  // namespace libredfish

#endif  // ECCLESIA_LIB_REDFISH_TOPOLOGY_H_
