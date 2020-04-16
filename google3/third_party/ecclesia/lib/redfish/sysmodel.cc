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

#include "lib/redfish/sysmodel.h"

#include <stddef.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "lib/redfish/interface.h"
#include "lib/redfish/property.h"
#include "lib/redfish/property_definitions.h"
#include "lib/redfish/result.h"
#include "lib/redfish/topology.h"
#include "lib/time/clock.h"

namespace libredfish {
namespace {

constexpr char kDevpathConnectorSuffix[] = "connector:";
constexpr char kDevpathDeviceSuffix[] = "device:";
constexpr char kDevpathRoot[] = "/phys";

constexpr char kPhysicalContextConnector[] = "Connector";

// If this Redfish object has a "Assembly" property, retrieve it and append it
// to assembly_out.
void ExtractAssemblyProperties(RedfishObject *obj,
                               std::vector<RedfishVariant> *assembly_out) {
  auto assembly_node = obj->GetNode(kRfPropertyAssembly).AsObject();
  if (!assembly_node) return;

  auto assembly_collection =
      assembly_node->GetNode(kRfPropertyAssemblies).AsIterable();
  if (!assembly_collection) return;

  for (auto assembly : *assembly_collection) {
    assembly_out->push_back(std::move(assembly));
  }
}

// Finds Assembly resources under the System URI by searching hardcoded URIs:
// * /redfish/v1/System/{system_id}/Memory/{memory_id}/Assembly
void ExtractAssemblyFromSystemUri(RedfishObject *root_obj,
                                  std::vector<RedfishVariant> *assembly_out) {
  auto system_collection = root_obj->GetNode(kRfPropertySystems).AsIterable();
  if (!system_collection) return;

  for (auto system : *system_collection) {
    auto system_obj = system.AsObject();
    if (!system_obj) continue;
    ExtractAssemblyProperties(system_obj.get(), assembly_out);

    auto memory_collection =
        system_obj->GetNode(kRfPropertyMemory).AsIterable();
    if (memory_collection) {
      for (auto memory : *memory_collection) {
        auto memory_obj = memory.AsObject();
        if (!memory_obj) continue;
        ExtractAssemblyProperties(memory_obj.get(), assembly_out);
      }
    }

    auto processors_collection =
        system_obj->GetNode(kRfPropertyProcessors).AsIterable();
    if (processors_collection) {
      for (auto processor : *processors_collection) {
        auto processor_obj = processor.AsObject();
        if (!processor_obj) continue;
        ExtractAssemblyProperties(processor_obj.get(), assembly_out);
      }
    }
  }
}

// Finds Assembly resources under the Chassis URI by searching hardcoded URIs:
// * /redfish/v1/chassis/{chassis_id}/Assembly
void ExtractAssemblyFromChassisUri(RedfishObject *root_obj,
                                   std::vector<RedfishVariant> *assembly_out) {
  auto chassis_collection = root_obj->GetNode(kRfPropertyChassis).AsIterable();
  if (!chassis_collection) return;

  for (auto chassis : *chassis_collection) {
    auto chassis_obj = chassis.AsObject();
    if (!chassis_obj) continue;
    ExtractAssemblyProperties(chassis_obj.get(), assembly_out);
  }
}

std::vector<RedfishVariant> FindAssemblyPayloads(
    RedfishInterface *redfish_intf) {
  std::vector<RedfishVariant> assemblies;
  auto root = redfish_intf->GetRoot().AsObject();
  if (!root) return assemblies;

  // TODO(dchanman): Make a much more general search algorithm. For now, just
  // search some hardcoded paths.
  ExtractAssemblyFromChassisUri(root.get(), &assemblies);
  ExtractAssemblyFromSystemUri(root.get(), &assemblies);

  return assemblies;
}

// Modifies the provided vector of Components by adding devpath information.
// Devpaths are generated from the provided upstream_connector_sequence and
// appending a suffix depending on the Component type.
void GenerateDevpathsForComponents(
    std::vector<std::unique_ptr<Component>> *components,
    absl::string_view upstream_connector_sequence) {
  for (auto &component : *components) {
    component->upstream_connector_sequence =
        std::string(upstream_connector_sequence);
    switch (component->type) {
      case Component::kBoard:
        component->local_devpath = std::string(upstream_connector_sequence);
        break;
      case Component::kConnector:
        component->local_devpath =
            absl::StrCat(upstream_connector_sequence, ":",
                         kDevpathConnectorSuffix, component->name);
        break;
      case Component::kDevice:
        component->local_devpath =
            absl::StrCat(upstream_connector_sequence, ":", kDevpathDeviceSuffix,
                         component->name);
        break;
    }
  }
}

// Assigns devpaths to all Components in the provided list of Assemblies.
// The input parameter assemblies will have its members modified with devpath
// information.
void GenerateDevpaths(std::vector<Assembly> *assemblies) {
  // Map @odata.id identifier to a Component. The Component pointers in this
  // map must have a local_devpath and upstream_connector_sequence assigned.
  // This map is used for downstream components to get their upstream connector
  // paths.
  absl::flat_hash_map<absl::string_view, const Component *>
      odata_id_to_component;
  // This set stores the Assembly structures which have already been processed.
  absl::flat_hash_set<Assembly *> processed;
  // last_processed_count will let us know if we are unable to process some
  // assemblies and we are stuck in the loop without making progress.
  size_t last_processed_count;
  do {
    last_processed_count = processed.size();
    // We need to make multiple passes through the list of Assemblies and assign
    // devpaths to components in a topological order, with the root Assemblies
    // first. Downstream assemblies cannot determine their devpaths without
    // their upstream connectors having a devpath.
    for (auto &assembly : *assemblies) {
      if (processed.count(&assembly)) continue;
      // If there is no upstream_odata_id value, this Assembly is the root.
      std::string upstream_connector_sequence = kDevpathRoot;
      if (assembly.upstream_odata_id.has_value()) {
        // Check that our upstream reference has had a devpath assigned to it.
        // If not, we have to skip assignment this iteration and try again
        // in a subsequent iteration.
        auto upstream_component =
            odata_id_to_component.find(assembly.upstream_odata_id.value());
        if (upstream_component == odata_id_to_component.end()) continue;
        upstream_connector_sequence = absl::StrCat(
            upstream_component->second->upstream_connector_sequence, "/",
            upstream_component->second->name);
      }
      // After new devpaths have been assigned, we need to update our upstream
      // mappings for odata.id to Component.
      GenerateDevpathsForComponents(&assembly.components,
                                    upstream_connector_sequence);
      for (const auto &component : assembly.components) {
        odata_id_to_component[component->odata_id] = component.get();
      }
      processed.insert(&assembly);
    }
  } while (processed.size() > last_processed_count);
}

// Parses a raw Redfish Assembly payload into an Assembly data structure.
// uri_to_associated_component_map will be modified with new mappings of URIs
// to Component instances which use the URI to source its Property data.
std::optional<Assembly> ProcessAssembly(
    PropertyRegistry *property_registry, ecclesia::Clock *clock,
    RedfishObject *assembly_payload,
    absl::flat_hash_map<std::string, std::vector<Component *>>
        *uri_to_associated_component_map) {
  Assembly assembly;

  auto fru_name = assembly_payload->GetNodeValue<PropertyName>();
  if (!fru_name.has_value()) return std::nullopt;
  assembly.name = fru_name.value();

  auto oem = assembly_payload->GetNode(kRfPropertyOem).AsObject();
  if (!oem) return assembly;

  auto google = oem->GetNode(kRfOemPropertyGoogle).AsObject();
  if (!google) return assembly;

  // Fetch the Assembly's upstream odata.id reference if it exists
  auto attached_to = google->GetNode(kRfOemPropertyAttachedTo).AsIterable();
  if (attached_to && !attached_to->Empty()) {
    auto primary_upstream = attached_to->GetIndex(0).AsObject();
    if (primary_upstream) {
      assembly.upstream_odata_id = primary_upstream->GetUri();
    }
  }

  auto components = google->GetNode(kRfOemPropertyComponents).AsIterable();
  if (!components) return assembly;

  for (auto component_resource : *components) {
    auto component_resource_obj = component_resource.AsObject();
    if (!component_resource_obj) continue;
    auto component = absl::make_unique<Component>();

    auto odata_id = component_resource_obj->GetNodeValue<PropertyOdataId>();
    if (!odata_id.has_value()) continue;
    component->odata_id = odata_id.value();

    auto component_name = component_resource_obj->GetNodeValue<PropertyName>();
    if (!component_name.has_value()) continue;
    component->name = component_name.value();

    // Determine the component type. Assume by default it is a device.
    component->type = Component::Type::kDevice;
    if (component->name == fru_name.value()) {
      // If the Component's name matches the Assembly's name, it is the board.
      component->type = Component::Type::kBoard;
    } else {
      // If the Component's PhysicalContext indicates it is a connector, it is
      // a connector.
      auto physical_context =
          component_resource_obj->GetNodeValue<PropertyPhysicalContext>();
      if (physical_context.has_value()) {
        if (physical_context.value() == kPhysicalContextConnector) {
          component->type = Component::Type::kConnector;
        }
      }
    }

    // For every resource associated with this Component, run
    // ExtractAllProperties to extract all relevant Redfish properties from the
    // resource into the Component's property container.
    auto associated_with =
        component_resource_obj->GetNode(kRfOemPropertyAssociatedWith);
    if (auto associated_with_iterable = associated_with.AsIterable()) {
      for (auto association : *associated_with_iterable) {
        if (auto association_obj = association.AsObject()) {
          property_registry->ExtractAllProperties(
              association_obj.get(), clock->Now(), &component->properties);
        }
      }
    }

    assembly.components.push_back(std::move(component));
  }

  return assembly;
}

// Run during initialization. Fetches all Assemblies from a Redfish server
// and parses them into data structs.
Topology CreateTopologyFromRedfish(RedfishInterface *redfish_intf,
                                   PropertyRegistry *property_registry,
                                   ecclesia::Clock *clock) {
  Topology topology;
  for (auto &assembly : FindAssemblyPayloads(redfish_intf)) {
    auto view = assembly.AsObject();
    if (!view) continue;
    auto processed_info =
        ProcessAssembly(property_registry, clock, view.get(),
                        &topology.uri_to_associated_component_map);
    if (processed_info.has_value()) {
      topology.assemblies.push_back(std::move(processed_info.value()));
    }
  }

  GenerateDevpaths(&topology.assemblies);

  for (const auto &assembly : topology.assemblies) {
    for (const auto &component : assembly.components) {
      topology.devpath_to_component_map[component->local_devpath] =
          component.get();
    }
  }
  return topology;
}

}  // namespace

Sysmodel::Sysmodel(RedfishInterface *redfish_intf,
                   PropertyRegistry *property_registry, ecclesia::Clock *clock)
    : Sysmodel(
          redfish_intf, property_registry, clock,
          CreateTopologyFromRedfish(redfish_intf, property_registry, clock)) {}

std::vector<std::string> Sysmodel::GetDevpaths() {
  std::vector<std::string> devpaths;
  for (const auto *component : GetAllComponents()) {
    devpaths.push_back(component->local_devpath);
  }
  return devpaths;
}

std::vector<Result<std::string>> Sysmodel::GetPlugins() {
  std::vector<Result<std::string>> plugins;
  for (const auto *component : GetAllComponents()) {
    if (component->type == Component::Type::kBoard) {
      plugins.push_back({component->local_devpath, component->name});
    }
  }
  return plugins;
}

}  // namespace libredfish
