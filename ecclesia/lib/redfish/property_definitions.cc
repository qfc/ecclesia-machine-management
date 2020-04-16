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

#include "lib/redfish/property_definitions.h"

#include <memory>

#include "lib/redfish/property.h"

namespace libredfish {

// Add new property definitions to this function
std::unique_ptr<PropertyRegistry> MakeDefaultPropertyRegistry() {
  auto registry = absl::make_unique<PropertyRegistry>();
  registry->Register<PropertyCapacityMiB>();
  registry->Register<PropertyLogicalSizeMiB>();
  registry->Register<PropertyManufacturer>();
  registry->Register<PropertyMemoryDeviceType>();
  registry->Register<PropertyOperatingSpeedMhz>();
  registry->Register<PropertyPartNumber>();
  registry->Register<PropertySerialNumber>();
  registry->Register<PropertyTotalCores>();
  registry->Register<PropertyTotalEnabledCores>();
  registry->Register<PropertyTotalThreads>();
  registry->Register<PropertyMaxSpeedMhz>();

  return registry;
}

}  // namespace libredfish
