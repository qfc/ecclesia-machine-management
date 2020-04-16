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

#ifndef ECCLESIA_LIB_REDFISH_SYSMODEL_H_
#define ECCLESIA_LIB_REDFISH_SYSMODEL_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "lib/redfish/interface.h"
#include "lib/redfish/property.h"
#include "lib/redfish/result.h"
#include "lib/redfish/topology.h"
#include "lib/time/clock.h"

namespace libredfish {

// This helper class uses a provided redfish_intf to construct the physical
// system toplogy and extract system information from Redfish Assembly
// resources.
class Sysmodel {
 public:
  // Constructor which will generate the topology by crawling the Redfish
  // resource tree and parsing all Assembly structures which are found.
  Sysmodel(RedfishInterface *redfish_intf, PropertyRegistry *property_registry,
           ecclesia::Clock *clock);
  // Constructor which allows Topology to be injected for testing.
  Sysmodel(RedfishInterface *redfish_intf, PropertyRegistry *property_registry,
           ecclesia::Clock *clock, Topology topology)
      : redfish_intf_(redfish_intf),
        property_registry_(property_registry),
        clock_(clock),
        topology_(std::move(topology)) {}
  Sysmodel(const Sysmodel &) = delete;
  Sysmodel &operator=(const Sysmodel &) = delete;

  std::vector<std::string> GetDevpaths();
  std::vector<Result<std::string>> GetPlugins();

  bool DevpathExists(absl::string_view devpath) const {
    return topology_.devpath_to_component_map.contains(devpath);
  }

  // The following Query definitions invokes result_callback with (devpath,
  // value) for Redfish Assembly Components providing the desired property.
  // When invoking the Query templates, only the PropertyDefinitionT must be
  // provided; CallbackF is intended to be inferred.
  //
  // CallbackF is a functor with signature void(PropertyDefinitionT::type r)
  // where r is a single Result<PropertyDefinitionT::type> for the query.

  // QueryAll will check all components in the system.
  template <typename PropertyDefinitionT, typename CallbackF>
  void QueryAll(const CallbackF &result_callback) {
    auto now = clock_->Now();
    for (auto *component : GetAllComponents()) {
      auto maybe_val = CachedRead<PropertyDefinitionT>(component, now);
      if (maybe_val.has_value()) {
        result_callback(Result<typename PropertyDefinitionT::type>{
            component->local_devpath, maybe_val.value()});
      }
    }
  }

  // Query only checks all components matching the provided devpaths.
  template <typename PropertyDefinitionT, typename CallbackF>
  void Query(absl::Span<const absl::string_view> devpaths,
             const CallbackF &result_callback) {
    auto now = clock_->Now();
    for (const auto &devpath : devpaths) {
      auto component = topology_.devpath_to_component_map.find(devpath);
      if (component == topology_.devpath_to_component_map.end()) {
        continue;
      }
      auto maybe_val = CachedRead<PropertyDefinitionT>(component->second, now);
      if (maybe_val.has_value()) {
        result_callback(Result<typename PropertyDefinitionT::type>{
            component->second->local_devpath, maybe_val.value()});
      }
    }
  }

 private:
  // CachedRead will try to retrieve the cached value of a given
  // PropertyDefinitionT from the provided component. The provided timestamp
  // (now) is used to determine if the read value is stale. If the value is
  // stale, the property will be updated by refetching the URI which provides
  // the data. As a side effect, all other components with properties derived
  // from the same URI will be updated as well. Returns a valid value if
  // successful. Returns nullopt if the component does not have the value or an
  // up-to-date value could not be retrieved.
  template <typename PropertyDefinitionT>
  absl::optional<typename PropertyDefinitionT::type> CachedRead(
      Component *component, absl::Time now) {
    auto result = component->properties.Get<PropertyDefinitionT>();
    if (result.has_value()) {
      auto property = result.value();
      if (property.IsStale(now)) {
        UpdateAllComponentsFromUri(property.SourceUri(), now);
        result = component->properties.Get<PropertyDefinitionT>();
        if (result.has_value() && !result.value().IsStale(now)) {
          return result.value().Value();
        }
      } else {
        return property.Value();
      }
    }
    return absl::nullopt;
  }

  void UpdateAllComponentsFromUri(absl::string_view uri, absl::Time now) {
    // TODO(b/145775513): Handle data degradation if GetUri returns bad data
    // or intermittently fails
    auto data = redfish_intf_->GetUri(uri);
    if (auto obj = data.AsObject()) {
      auto affected_components =
          topology_.uri_to_associated_component_map.find(uri);
      if (affected_components !=
          topology_.uri_to_associated_component_map.end()) {
        for (auto *component : affected_components->second) {
          property_registry_->ExtractAllProperties(obj.get(), now,
                                                   &component->properties);
        }
      }
    }
  }

  // Helper method to get all Components from all Assemblies.
  std::vector<Component *> GetAllComponents() const {
    std::vector<Component *> components;
    for (const auto &assembly : topology_.assemblies) {
      for (auto &component : assembly.components) {
        components.push_back(component.get());
      }
    }
    return components;
  }

  RedfishInterface *redfish_intf_;
  PropertyRegistry *property_registry_;
  ecclesia::Clock *clock_;
  Topology topology_;
};

}  // namespace libredfish

#endif  // ECCLESIA_LIB_REDFISH_SYSMODEL_H_
