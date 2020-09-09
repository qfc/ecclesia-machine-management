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

#include "ecclesia/magent/redfish/indus/redfish_service.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "ecclesia/lib/logging/logging.h"
#include "ecclesia/magent/redfish/core/assembly.h"
#include "ecclesia/magent/redfish/core/redfish_keywords.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "ecclesia/magent/redfish/indus/chassis.h"
#include "ecclesia/magent/redfish/indus/firmware_inventory.h"
#include "ecclesia/magent/redfish/indus/memory.h"
#include "ecclesia/magent/redfish/indus/memory_collection.h"
#include "ecclesia/magent/redfish/indus/memory_metrics.h"
#include "ecclesia/magent/redfish/indus/processor.h"
#include "ecclesia/magent/redfish/indus/processor_collection.h"
#include "ecclesia/magent/redfish/indus/processor_metrics.h"
#include "ecclesia/magent/redfish/indus/root.h"
#include "ecclesia/magent/redfish/indus/service_root.h"
#include "ecclesia/magent/redfish/indus/software.h"
#include "ecclesia/magent/redfish/indus/software_inventory.h"
#include "ecclesia/magent/redfish/indus/system.h"
#include "ecclesia/magent/redfish/indus/systems.h"
#include "ecclesia/magent/redfish/indus/thermal.h"
#include "ecclesia/magent/redfish/indus/update_service.h"
#include "ecclesia/magent/sysmodel/x86/fru.h"
#include "ecclesia/magent/sysmodel/x86/sysmodel.h"
#include "json/value.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"

namespace ecclesia {

namespace {

// This helper function creates an AssemblyModifier that can be used to add some
// sysmodel FRU info (e.g., part and serial numbers) to the assemblies resource.
// For example, CreateModifierToAddFruInfo(fru1,
// "/redfish/v1/Chassis/Sleipnir/Assembly", "bmc_riser") returns an
// AssemblyModifier which will find the assembly resource with URL
// "/redfish/v1/Chassis/Sleipnir/Assembly" and add part & serial numbers of fru1
// to the component named "bmc_riser".
Assembly::AssemblyModifier CreateModifierToAddFruInfo(
    const SysmodelFru &sysmodel_fru, const std::string &assembly_url,
    const std::string &component_name) {
  return [&](absl::flat_hash_map<std::string, Json::Value> &assemblies) {
    auto assembly_iter = assemblies.find(assembly_url);
    if (assembly_iter == assemblies.end()) {
      ErrorLog() << "Failed to find a matched asembly with URL: "
                 << assembly_url;
      return;
    }
    auto &assembly_resource_json = assembly_iter->second;
    for (auto &assembly : assembly_resource_json[kAssemblies]) {
      if (assembly[kName] == component_name) {
        assembly[kPartNumber] = std::string(sysmodel_fru.GetPartNumber());
        assembly[kSerialNumber] = std::string(sysmodel_fru.GetSerialNumber());
        return;
      }
    }
  };
}
}  // namespace

IndusRedfishService::IndusRedfishService(HTTPServerInterface *server,
                                         SystemModel *system_model,
                                         absl::string_view assemblies_dir) {
  resources_.push_back(CreateResource<Root>(server));
  resources_.push_back(CreateResource<ServiceRoot>(server));
  resources_.push_back(CreateResource<ComputerSystemCollection>(server));
  resources_.push_back(CreateResource<ComputerSystem>(server));
  resources_.push_back(CreateResource<ChassisCollection>(server, system_model));
  resources_.push_back(CreateResource<Chassis>(server, system_model));
  resources_.push_back(CreateResource<MemoryCollection>(server, system_model));
  resources_.push_back(CreateResource<Memory>(server, system_model));

  // Create a vector of AssemblyModifier that add the IPMI FRUs part number and
  // serial number to the corresponding component in Sleipnir assemblies.
  std::vector<Assembly::AssemblyModifier> assembly_modifiers;
  const std::string sleipnir_chassis_assembly_url =
      "/redfish/v1/Chassis/Sleipnir/Assembly";
  if (auto *fru_reader = system_model->GetFruReader("sleipnir_hsbp")) {
    auto optional_fru = fru_reader->Read();
    if (optional_fru.has_value()) {
      assembly_modifiers.push_back(CreateModifierToAddFruInfo(
          optional_fru.value(), sleipnir_chassis_assembly_url,
          "sleipnir_mainboard"));
    }
  }
  if (auto *fru_reader = system_model->GetFruReader("sleipnir_bmc")) {
    auto optional_fru = fru_reader->Read();
    if (optional_fru.has_value()) {
      assembly_modifiers.push_back(CreateModifierToAddFruInfo(
          optional_fru.value(), sleipnir_chassis_assembly_url, "bmc_riser"));
    }
  }

  resources_.push_back(CreateResource<Assembly>(server, assemblies_dir,
                                                std::move(assembly_modifiers)));
  resources_.push_back(CreateResource<MemoryMetrics>(server, system_model));
  resources_.push_back(
      CreateResource<ProcessorCollection>(server, system_model));

  resources_.push_back(CreateResource<Processor>(server, system_model));
  resources_.push_back(CreateResource<ProcessorMetrics>(server, system_model));
  resources_.push_back(CreateResource<Thermal>(server, system_model));
  resources_.push_back(CreateResource<UpdateService>(server));
  resources_.push_back(CreateResource<SoftwareInventoryCollection>(server));
  resources_.push_back(CreateResource<SoftwareInventory>(server));
  resources_.push_back(CreateResource<FirmwareInventoryCollection>(server));
}

}  // namespace ecclesia
