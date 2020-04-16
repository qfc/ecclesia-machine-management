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

#ifndef ECCLESIA_LIB_REDFISH_ASSEMBLY_H_
#define ECCLESIA_LIB_REDFISH_ASSEMBLY_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/types/optional.h"
#include "lib/redfish/property.h"

namespace libredfish {

// Component is a data structure containing processed data derived from Redfish
// properties. At a high level, Component represents a physical node in the
// system (e.g. board, connector, device).
struct Component {
  // Redfish name property value
  std::string name;
  // Redfish @odata.id property value is a Redfish URI identifier
  std::string odata_id;
  // Type is used in the devpath generation algorithm.
  enum Type { kBoard, kConnector, kDevice };
  Type type;

  // This devpath is the physical identifier specifically in the domain of the
  // associated Redfish backend. This devpath should not be confused with the
  // global devpath as seen and translated at the Machine Master level.
  std::string local_devpath;
  // This string is the upstream connector sequence (all connector names
  // concatenated with "/") of the plugin this component resides on,
  // specifically in the domain of the associated Redfish backend. This
  // connector sequence will not reference any connectors in other domains
  // visible at the Machine Master level.
  std::string upstream_connector_sequence;

  // Interface for accessing all properties associated with this component.
  PropertyContainer properties;
};

// Assembly is a data structure containing processed data derived from Redfish
// properties. At a high level, Assembly represents a field replaceable unit
// comprised of a collection of Components.
struct Assembly {
  // Redfish name property value
  std::string name;
  // The constituent components in this Assembly
  std::vector<std::unique_ptr<Component>> components;
  // Redfish @odata.id of the upstream component
  absl::optional<std::string> upstream_odata_id;
};

// At a high level, Topology represents the entire system presented by a
// Redfish service comprised of a collection of Assemblies.
struct Topology {
  // All Assemblies found from the Redfish backend.
  std::vector<Assembly> assemblies;

  // All member variables below are simply views of assemblies to facilitate
  // common data queries. Note that assemblies must NOT be modified in a way
  // which invalidates these views.

  // Map of a URI to all Components which scrape property data from it.
  absl::flat_hash_map<std::string, std::vector<Component *>>
      uri_to_associated_component_map;
  absl::flat_hash_map<std::string, Component *> devpath_to_component_map;
};

}  // namespace libredfish

#endif  // ECCLESIA_LIB_REDFISH_ASSEMBLY_H_
