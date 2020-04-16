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

// Common property definitions.
//
// PropertyDefinition subclasses are to be defined in this header to be used
// with the utilities in property.h (e.g. PropertyContainer, PropertyRegistry).
//
// Properties that wish to be automatically extracted from Assembly-Component
// resources must be registered to a PropertyRegistry.
// MakeDefaultPropertyRegistry provides a place for properties to be registered
// for typical usage. Most libraries should opt to use the registry created
// by MakeDefaultPropertyRegistry if they are extracting properties in the
// ususal case.
#ifndef ECCLESIA_LIB_REDFISH_PROPERTY_DEFINITIONS_H_
#define ECCLESIA_LIB_REDFISH_PROPERTY_DEFINITIONS_H_

#include <memory>
#include <string>

#include "absl/time/time.h"
#include "lib/redfish/property.h"

namespace libredfish {

// String definitions of property fields to search programmatically. These
// are separate from the PropertyDefinitions for now as we cannot define the
// Redfish types "Array" and "Object" at the moment.
// TODO(b/144313582): Fold this into PropertyRegistry data objects

inline constexpr char kRfPropertyAssemblies[] = "Assemblies";
inline constexpr char kRfPropertyAssembly[] = "Assembly";
inline constexpr char kRfPropertyChassis[] = "Chassis";
inline constexpr char kRfPropertyMemory[] = "Memory";
inline constexpr char kRfPropertyOem[] = "Oem";
inline constexpr char kRfPropertyProcessors[] = "Processors";
inline constexpr char kRfPropertySystems[] = "Systems";

inline constexpr char kRfOemPropertyAssociatedWith[] = "AssociatedWith";
inline constexpr char kRfOemPropertyAttachedTo[] = "AttachedTo";
inline constexpr char kRfOemPropertyGoogle[] = "Google";
inline constexpr char kRfOemPropertyComponents[] = "Components";

// Returns a new PropertyRegistry instance with a default set of properties
// and extraction methods defined.
std::unique_ptr<PropertyRegistry> MakeDefaultPropertyRegistry();

DEFINE_REDFISH_PROPERTY(PropertyOdataId, std::string, "@odata.id",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyCapacityMiB, int, "CapacityMiB",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyLogicalSizeMiB, int, "LogicalSizeMiB",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyManufacturer, std::string, "Manufacturer",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyMemoryDeviceType, std::string,
                        "MemoryDeviceType", absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyName, std::string, "Name",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyOperatingSpeedMhz, int, "OperatingSpeedMhz",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyPartNumber, std::string, "PartNumber",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyPhysicalContext, std::string, "PhysicalContext",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertySerialNumber, std::string, "SerialNumber",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyTotalCores, int, "TotalCores",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyTotalEnabledCores, int, "TotalEnabledCores",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyTotalThreads, int, "TotalThreads",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(PropertyMaxSpeedMhz, int, "MaxSpeedMHz",
                        absl::InfiniteDuration());

}  // namespace libredfish

#endif  // ECCLESIA_LIB_REDFISH_PROPERTY_DEFINITIONS_H_
