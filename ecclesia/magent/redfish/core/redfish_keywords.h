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

#ifndef ECCLESIA_MAGENT_REDFISH_CORE_REDFISH_KEYWORDS_H_
#define ECCLESIA_MAGENT_REDFISH_CORE_REDFISH_KEYWORDS_H_

namespace ecclesia {

// Redfish keywords
inline constexpr char kOdataId[] = "@odata.id";
inline constexpr char kOdataType[] = "@odata.type";
inline constexpr char kOdataContext[] = "@odata.context";
inline constexpr char kId[] = "Id";
inline constexpr char kName[] = "Name";
inline constexpr char kSystems[] = "Systems";
inline constexpr char kChassis[] = "Chassis";
inline constexpr char kManagers[] = "Managers";
inline constexpr char kUpdateService[] = "UpdateService";
inline constexpr char kMembers[] = "Members";
inline constexpr char kMembersCount[] = "Members@odata.count";
inline constexpr char kOem[] = "Oem";
inline constexpr char kGoogle[] = "Google";
inline constexpr char kMemory[] = "Memory";
inline constexpr char kStorage[] = "Storage";
inline constexpr char kDrives[] = "Drives";
inline constexpr char kStatus[] = "Status";
inline constexpr char kState[] = "State";
inline constexpr char kDimm[] = "DIMM";
inline constexpr char kPartNumber[] = "PartNumber";
inline constexpr char kSerialNumber[] = "SerialNumber";
inline constexpr char kLinks[] = "Links";
inline constexpr char kPCIeDevices[] = "PCIeDevices";
inline constexpr char kPCIeInterface[] = "PCIeInterface";
inline constexpr char kPCIeFunctions[] = "PCIeFunctions";
inline constexpr char kManufacturer[] = "Manufacturer";
inline constexpr char kCapacityMiB[] = "CapacityMiB";
inline constexpr char kLogicalSizeMiB[] = "LogicalSizeMiB";
inline constexpr char kMemoryDeviceType[] = "MemoryDeviceType";
inline constexpr char kOperatingSpeedMhz[] = "OperatingSpeedMhz";
inline constexpr char kAssembly[] = "Assembly";
inline constexpr char kMemoryErrorCounts[] = "MemoryErrorCounts";
inline constexpr char kCorrectable[] = "Correctable";
inline constexpr char kUncorrectable[] = "Uncorrectable";
inline constexpr char kMetrics[] = "Metrics";
inline constexpr char kMemoryMetrics[] = "MemoryMetrics";
inline constexpr char kMaxSpeedMHz[] = "MaxSpeedMHz";
inline constexpr char kTotalCores[] = "TotalCores";
inline constexpr char kTotalEnabledCores[] = "TotalEnabledCores";
inline constexpr char kTotalThreads[] = "TotalThreads";
inline constexpr char kProcessorId[] = "ProcessorId";
inline constexpr char kEffectiveFamily[] = "EffectiveFamily";
inline constexpr char kEffectiveModel[] = "EffectiveModel";
inline constexpr char kStep[] = "Step";
inline constexpr char kSocket[] = "Socket";
inline constexpr char kVendorId[] = "VendorId";
inline constexpr char kProcessorMetrics[] = "ProcessorMetrics";
inline constexpr char kProcessorErrorCounts[] = "ProcessorErrorCounts";
inline constexpr char kProcessors[] = "Processors";
inline constexpr char kSystemType[] = "SystemType";
inline constexpr char kAssetTag[] = "AssetTag";
inline constexpr char kModel[] = "Model";
inline constexpr char kSKU[] = "SKU";
inline constexpr char kUUID[] = "UUID";
inline constexpr char kHealth[] = "Health";
inline constexpr char kIndicatorLED[] = "IndicatorLED";
inline constexpr char kPowerState[] = "PowerState";
inline constexpr char kBootSourceOverrideEnabled[] =
    "BootSourceOverrideEnabled";
inline constexpr char kBootSourceOverrideMode[] = "BootSourceOverrideMode";
inline constexpr char kBiosVersion[] = "BiosVersion";
inline constexpr char kCount[] = "Count";
inline constexpr char kTotalSystemMemoryGiB[] = "TotalSystemMemoryGiB";
inline constexpr char kBootSourceOverrideTarget[] = "BootSourceOverrideTarget";
inline constexpr char kUefiTargetBootSourceOverride[] =
    "UefiTargetBootSourceOverride";
inline constexpr char kTarget[] = "target";
inline constexpr char kBoot[] = "Boot";
inline constexpr char kProcessorSummary[] = "ProcessorSummary";
inline constexpr char kMemorySummary[] = "MemorySummary";
inline constexpr char kActions[] = "Actions";
inline constexpr char kComputerSystemReset[] = "#ComputerSystem.Reset";
inline constexpr char kResetTypeAllowableValues[] =
    "ResetType@Redfish.AllowableValues";
inline constexpr char kThermal[] = "Thermal";
inline constexpr char kPower[] = "Power";
inline constexpr char kComputerSystems[] = "ComputerSystems";
inline constexpr char kPowerControl[] = "PowerControl";
inline constexpr char kPowerLimit[] = "PowerLimit";
inline constexpr char kPowerConsumedWatts[] = "PowerConsumedWatts";
inline constexpr char kPowerCapacityWatts[] = "PowerCapacityWatts";
inline constexpr char kLimitInWatts[] = "LimitInWatts";
inline constexpr char kLimitException[] = "LimitException";
inline constexpr char kReadingCelsius[] = "ReadingCelsius";
inline constexpr char kUpperThresholdNonCritical[] =
    "UpperThresholdNonCritical";
inline constexpr char kUpperThresholdCritical[] = "UpperThresholdCritical";
inline constexpr char kUpperThresholdFatal[] = "UpperThresholdFatal";
inline constexpr char kPhysicalContext[] = "PhysicalContext";
inline constexpr char kTemperatures[] = "Temperatures";
inline constexpr char kEthernetIntf[] = "EthernetInterfaces";
inline constexpr char kRdmaSupported[] = "RdmaSupported";
inline constexpr char kMACAddress[] = "MACAddress";
inline constexpr char kLinkStatus[] = "LinkStatus";
inline constexpr char kSpeedMbps[] = "SpeedMbps";
inline constexpr char kReading[] = "Reading";
inline constexpr char kReadingUnits[] = "ReadingUnits";
inline constexpr char kMinReadingRange[] = "MinReadingRange";
inline constexpr char kMaxReadingRange[] = "MaxReadingRange";
inline constexpr char kRelatedItem[] = "RelatedItem";
inline constexpr char kEnabled[] = "Enabled";
inline constexpr char kAbsent[] = "Absent";
inline constexpr char kThrottlingCelsius[] = "ThrottlingCelsius";
inline constexpr char kMemberId[] = "MemberId";
inline constexpr char kVersion[] = "Version";
inline constexpr char kSoftwareInventory[] = "SoftwareInventory";
inline constexpr char kFirmwareInventory[] = "FirmwareInventory";

// Assembly keywords
inline constexpr char kAssemblies[] = "Assemblies";
inline constexpr char kAssemblyDataType[] = "#Assembly.v1_2_1.Assembly";
inline constexpr char kComponents[] = "Components";

// Google Oem Keywords
inline constexpr char kAssociatedWith[] = "AssociatedWith";
inline constexpr char kAttachedTo[] = "AttachedTo";
inline constexpr char kDevpath[] = "Devpath";

// Redfish resource URIs
inline constexpr char kServiceRootUri[] = "/redfish/v1";
inline constexpr char kComputerSystemCollectionUri[] = "/redfish/v1/Systems";
inline constexpr char kComputerSystemUri[] = "/redfish/v1/Systems/system";
inline constexpr char kChassisCollectionUri[] = "/redfish/v1/Chassis";
inline constexpr char kChassisUri[] = "/redfish/v1/Chassis/chassis";
inline constexpr char kMemoryCollectionUri[] =
    "/redfish/v1/Systems/system/Memory";
inline constexpr char kMemoryUriPattern[] =
    "/redfish/v1/Systems/system/Memory/(\\d+)";
inline constexpr char kMemoryMetricsUriPattern[] =
    "/redfish/v1/Systems/system/Memory/(\\d+)/MemoryMetrics";
// Regex pattern for an Assembly resource.
inline constexpr char kAssemblyUriPattern[] = "/redfish/v1/[\\w/]+/Assembly";
inline constexpr char kProcessorCollectionUri[] =
    "/redfish/v1/Systems/system/Processors";
inline constexpr char kProcessorUriPattern[] =
    "/redfish/v1/Systems/system/Processors/(\\d+)";
inline constexpr char kProcessorMetricsUriPattern[] =
    "/redfish/v1/Systems/system/Processors/(\\d+)/ProcessorMetrics";
inline constexpr char kThermalUri[] = "/redfish/v1/Chassis/chassis/Thermal";
inline constexpr char kPowerUri[] = "/redfish/v1/Chassis/chassis/Power";
inline constexpr char kAssemblyUri[] = "/redfish/v1/Chassis/chassis/Assembly";
inline constexpr char kPowerControlUri[] =
    "/redfish/v1/Chassis/chassis/Power#/PowerControl/0";
inline constexpr char kTemperaturesUri[] =
    "/redfish/v1/Chassis/chassis/Thermal#/Temperatures/0";
inline constexpr char kEthernetInterfaceCollectionUri[] =
    "/redfish/v1/Systems/system/EthernetInterfaces";
inline constexpr char kEthernetInterfaceUri[] =
    "/redfish/v1/Systems/system/EthernetInterfaces/0";
inline constexpr char kStorageCollectionUri[] =
    "/redfish/v1/Systems/system/Storage";
inline constexpr char kDrivesCollectionUri[] =
    "/redfish/v1/Systems/system/Storage/storage/Drives";
inline constexpr char kUpdateServiceUri[] = "/redfish/v1/UpdateService";
inline constexpr char kSoftwareInventoryCollectionUri[] =
    "/redfish/v1/UpdateService/SoftwareInventory";
inline constexpr char kFirmwareInventoryCollectionUri[] =
    "/redfish/v1/UpdateService/FirmwareInventory";
inline constexpr char kSoftwareInventoryMagentUri[] =
    "/redfish/v1/UpdateService/SoftwareInventory/magent";
inline constexpr char kFirmwareInventoryMagentUri[] =
    "/redfish/v1/UpdateService/FirmwareInventory/magent";

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_CORE_REDFISH_KEYWORDS_H_
