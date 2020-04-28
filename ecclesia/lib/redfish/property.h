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

#ifndef ECCLESIA_LIB_REDFISH_PROPERTY_H_
#define ECCLESIA_LIB_REDFISH_PROPERTY_H_

#include <string>
#include <tuple>
#include <utility>

#include "base/logging.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "lib/redfish/interface.h"

namespace libredfish {

inline constexpr absl::string_view kRfPropertyOdataTypeName = "@odata.type";

// PropertyContainer is a hashmap which stores a variant. This class's purpose
// is to abstract the typed Set and Get operations which are used with the
// Property definitions. The class also stores caching metadata in order for
// users to determine that the stored values are stale. The class itself does
// not implement the mechanism for refreshing cached values, but provides
// sufficient information for the caller to detect cache staleness and update
// the stored value.
//
// PropertyContainer can store multiple properties sourced from multiple
// resources. For example, a "PartNumber" property can be sourced from a
// "Memory" resource as well as a "Processor" resource. The PropertyContainer
// API will provide mechanisms for specifying the desired resource.
class PropertyContainer {
 public:
  // Set only supports PropertyDefinitionT types which are one of the underlying
  // supported types in the hashmap implementation.
  template <typename PropertyDefinitionT>
  void Set(typename PropertyDefinitionT::type value, std::string resource,
           std::string source_uri, const absl::Time &collection_time) {
    properties_[resource][PropertyDefinitionT::Name] = InternalCachedValue{
        value, std::move(source_uri),
        collection_time + PropertyDefinitionT::CacheInterval};
  }

  // CachedValue is a data class for storing strongly typed value and cache
  // metadata as a return value for PropertyContainer queries.
  template <typename T>
  class CachedValue {
   public:
    CachedValue(T value, std::string source_uri, absl::Time expiration_time)
        : value_(value),
          source_uri_(source_uri),
          expiration_time_(expiration_time) {}
    bool operator==(const CachedValue<T> &other) const {
      return std::tie(value_, source_uri_, expiration_time_) ==
             std::tie(other.value_, other.source_uri_, other.expiration_time_);
    }
    bool operator!=(const CachedValue<T> &other) const {
      return *this != other;
    }
    // Returns the value.
    const T &Value() { return value_; }

    // Returns a Redfish URI where the value can be updated.
    absl::string_view SourceUri() { return source_uri_; }
    // Returns true if the value is stale at the provided time.
    bool IsStale(absl::Time now) { return now > expiration_time_; }

   private:
    T value_;
    std::string source_uri_;
    absl::Time expiration_time_;
  };

  // Get returns the typed CachedValue of PropertyDefinitionT from the
  // PropertyContainer scraped from all ResourceDefinitionT. nullopt is returned
  // if the PropertyDefinitionT does not have a stored value.
  template <typename ResourceDefinitionT, typename PropertyDefinitionT>
  absl::optional<CachedValue<typename PropertyDefinitionT::type>> Get() const {
    auto resource_itr = properties_.find(ResourceDefinitionT::Name);
    if (resource_itr == properties_.end()) return absl::nullopt;

    auto property_itr = resource_itr->second.find(PropertyDefinitionT::Name);
    if (property_itr == resource_itr->second.end()) return absl::nullopt;
    const InternalCachedValue &cached_value = property_itr->second;

    if (!absl::holds_alternative<typename PropertyDefinitionT::type>(
            cached_value.value)) {
      LOG(DFATAL) << "Type mismatch for property: "
                  << PropertyDefinitionT::Name;
      return absl::nullopt;
    }

    return CachedValue<typename PropertyDefinitionT::type>{
        absl::get<typename PropertyDefinitionT::type>(cached_value.value),
        cached_value.source_uri, cached_value.expiration_time};
  }

 private:
  // All PropertyDefinition types must be one of the possible types in
  // PropertyVariant.
  using PropertyVariant = absl::variant<int, bool, std::string, double>;

  // Internal representation of a property which uses the PropertyVariant
  // type under the hood.
  struct InternalCachedValue {
    PropertyVariant value;
    std::string source_uri;
    absl::Time expiration_time;
  };

  // Properties are stored in two-level hashmap: map[ResourceName][Property]
  // ResourceName keys should be accessed by resources defined by the
  // DEFINE_REDFISH_RESOURCE macro. Property keys should be accessed by
  // resources defined by the DEFINE_REDFISH_PROPERTY macro.
  absl::flat_hash_map<std::string,
                      absl::flat_hash_map<std::string, InternalCachedValue>>
      properties_;
};

// ExtractIntoContainer extracts the value of the Property PropertyDefinitionT
// from object and stores it into container, with collection_time used to
// determine the cache expiry time of the value.
template <typename PropertyDefinitionT>
void ExtractIntoContainer(RedfishObject *object, PropertyContainer *container,
                          const absl::Time &collection_time) {
  auto maybe_type = object->GetNodeValue<std::string>(kRfPropertyOdataTypeName);
  std::string resource;
  if (maybe_type.has_value()) {
    absl::string_view last_elem;
    for (absl::string_view split : absl::StrSplit(*maybe_type, '.')) {
      last_elem = split;
    }
    resource = std::string(last_elem);
  }

  auto maybe_val = object->GetNodeValue<typename PropertyDefinitionT::type>(
      PropertyDefinitionT::Name);
  if (maybe_val.has_value()) {
    if (!object->GetUri().has_value()) {
      LOG(WARNING) << "Property " << PropertyDefinitionT::Name
                   << " is being collected from an object with no URI. Its "
                      "value will not be able to be fetched again.";
    }
    container->Set<PropertyDefinitionT>(
        std::move(maybe_val.value()), std::move(resource),
        std::move(object->GetUri().value_or("")), collection_time);
  }
}

class PropertyRegistry {
 public:
  // ExtractFunction is the function signature for a method which extracts one
  // named property from a Redfish object and stores it in the
  // PropertyContainer.
  using ExtractFunction = void (*)(RedfishObject *, PropertyContainer *,
                                   const absl::Time &collection_time);
  // Template helper for registering a default method of extraction
  template <typename PropertyDefinitionT>
  void Register() {
    Register(PropertyDefinitionT::Name,
             ExtractIntoContainer<PropertyDefinitionT>);
  }

  template <typename PropertyDefinitionT>
  void Register(ExtractFunction f) {
    Register(PropertyDefinitionT::Name, f);
  }

  // Extracts all registered properties from object into container.
  void ExtractAllProperties(RedfishObject *object,
                            const absl::Time &collection_time,
                            PropertyContainer *container);

 private:
  // Registers f as an extraction function for the given Property name.
  void Register(absl::string_view name, ExtractFunction f);

  absl::flat_hash_map<std::string, ExtractFunction> extract_func_map_;
};

// The base class for all property definitions. Subclasses must declare a type
// and a string Name.
template <typename PropertyDefinitionSubtypeT, typename PropertyType>
struct PropertyDefinition {
  using type = PropertyType;
};

// Macro for defining a Redfish property.
//
// This macro creates a PropertyDefinition subclass with the required static
// members to satisfy the CRTP expectations for new property definitions.
#define DEFINE_REDFISH_PROPERTY(classname, type, property_name,     \
                                cache_interval)                     \
  struct classname : PropertyDefinition<classname, type> {          \
    static constexpr char Name[] = property_name;                   \
    static constexpr absl::Duration CacheInterval = cache_interval; \
  }

// Macro for defining a Redfish resource.
//
// This macro creates a ResourceDefinition subclass with the required static
// members to satisfy the static member expectations for resource definitions.
#define DEFINE_REDFISH_RESOURCE(classname, resource_name) \
  struct classname {                                      \
    static constexpr char Name[] = resource_name;         \
  }
}  // namespace libredfish

#endif  // ECCLESIA_LIB_REDFISH_PROPERTY_H_
