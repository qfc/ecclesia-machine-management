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
// with typed extraction methods.
#ifndef ECCLESIA_LIB_REDFISH_PROPERTY_DEFINITIONS_H_
#define ECCLESIA_LIB_REDFISH_PROPERTY_DEFINITIONS_H_

#include <string>

#include "ecclesia/lib/redfish/property.h"

namespace libredfish {

// String definitions of property fields to search programmatically. These
// are separate from the PropertyDefinitions for now as we cannot define the
// Redfish types "Array" and "Object" at the moment.

inline constexpr char kRfPropertyAssemblies[] = "Assemblies";
inline constexpr char kRfPropertyAssembly[] = "Assembly";
inline constexpr char kRfPropertyChassis[] = "Chassis";
inline constexpr char kRfPropertyMemory[] = "Memory";
inline constexpr char kRfPropertyStorage[] = "Storage";
inline constexpr char kRfPropertyDrives[] = "Drives";
inline constexpr char kRfPropertyOem[] = "Oem";
inline constexpr char kRfPropertyProcessors[] = "Processors";
inline constexpr char kRfPropertyMetrics[] = "Metrics";
inline constexpr char kRfPropertySystems[] = "Systems";
inline constexpr char kRfPropertyEthernetInterfaces[] = "EthernetInterfaces";
inline constexpr char kRfPropertyThermal[] = "Thermal";
inline constexpr char kRfPropertyTemperatures[] = "Temperatures";
inline constexpr char kRfPropertyRelatedItem[] = "RelatedItem";
inline constexpr char kRfPropertyStatus[] = "Status";
inline constexpr char kRfPropertyState[] = "State";

inline constexpr char kRfOemPropertyAssociatedWith[] = "AssociatedWith";
inline constexpr char kRfOemPropertyAttachedTo[] = "AttachedTo";
inline constexpr char kRfOemPropertyGoogle[] = "Google";
inline constexpr char kRfOemPropertyComponents[] = "Components";
inline constexpr char kRfOemPropertyMemoryErrorCounts[] = "MemoryErrorCounts";
inline constexpr char kRfOemPropertyProcessorErrorCounts[] =
    "ProcessorErrorCounts";

DEFINE_REDFISH_RESOURCE(ResourceChassis, "Chassis");
DEFINE_REDFISH_RESOURCE(ResourceMemory, "Memory");
DEFINE_REDFISH_RESOURCE(ResourceDrive, "Drive");
DEFINE_REDFISH_RESOURCE(ResourceProcessor, "Processor");
DEFINE_REDFISH_RESOURCE(ResourceEthernetInterface, "EthernetInterface");
DEFINE_REDFISH_RESOURCE(ResourceTemperature, "Temperature");

DEFINE_REDFISH_PROPERTY(PropertyOdataId, std::string, "@odata.id");
DEFINE_REDFISH_PROPERTY(PropertyOdataType, std::string, "@odata.type");
DEFINE_REDFISH_PROPERTY(PropertyCapacityMiB, int, "CapacityMiB");
DEFINE_REDFISH_PROPERTY(PropertyLogicalSizeMiB, int, "LogicalSizeMiB");
DEFINE_REDFISH_PROPERTY(PropertyManufacturer, std::string, "Manufacturer");
DEFINE_REDFISH_PROPERTY(PropertyMemoryDeviceType, std::string,
                        "MemoryDeviceType");
DEFINE_REDFISH_PROPERTY(PropertyName, std::string, "Name");
DEFINE_REDFISH_PROPERTY(PropertyOperatingSpeedMhz, int, "OperatingSpeedMhz");
DEFINE_REDFISH_PROPERTY(PropertyPartNumber, std::string, "PartNumber");
DEFINE_REDFISH_PROPERTY(PropertyPhysicalContext, std::string,
                        "PhysicalContext");
DEFINE_REDFISH_PROPERTY(PropertySerialNumber, std::string, "SerialNumber");
DEFINE_REDFISH_PROPERTY(PropertyTotalCores, int, "TotalCores");
DEFINE_REDFISH_PROPERTY(PropertyTotalEnabledCores, int, "TotalEnabledCores");
DEFINE_REDFISH_PROPERTY(PropertyTotalThreads, int, "TotalThreads");
DEFINE_REDFISH_PROPERTY(PropertyMaxSpeedMhz, int, "MaxSpeedMHz");
DEFINE_REDFISH_PROPERTY(PropertyLinkStatus, std::string, "LinkStatus");
DEFINE_REDFISH_PROPERTY(PropertySpeedMbps, int, "SpeedMbps");
DEFINE_REDFISH_PROPERTY(PropertyMacAddress, std::string, "MACAddress");
DEFINE_REDFISH_PROPERTY(PropertyCapacityBytes, int64_t, "CapacityBytes");
DEFINE_REDFISH_PROPERTY(PropertyBlockSizeBytes, int, "BlockSizeBytes");
DEFINE_REDFISH_PROPERTY(PropertyMediaType, std::string, "MediaType");
DEFINE_REDFISH_PROPERTY(PropertyReadingCelsius, int, "ReadingCelsius");
DEFINE_REDFISH_PROPERTY(PropertyThrottlingCelsius, int, "ThrottlingCelsius");
DEFINE_REDFISH_PROPERTY(PropertyUpperThresholdCritical, int,
                        "UpperThresholdCritical");
DEFINE_REDFISH_PROPERTY(PropertyState, std::string, "State");

// OEM Google properties
DEFINE_REDFISH_PROPERTY(OemGooglePropertyCorrectable, int, "Correctable");
DEFINE_REDFISH_PROPERTY(OemGooglePropertyUncorrectable, int, "Uncorrectable");
}  // namespace libredfish

#endif  // ECCLESIA_LIB_REDFISH_PROPERTY_DEFINITIONS_H_
