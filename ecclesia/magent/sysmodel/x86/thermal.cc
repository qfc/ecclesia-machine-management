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

#include "ecclesia/magent/sysmodel/x86/thermal.h"

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "ecclesia/magent/lib/io/pci_sys.h"

namespace ecclesia {

PciThermalSensor::PciThermalSensor(const PciSensorParams &params)
    : ThermalSensor(params.name, params.upper_threshold_critical),
      offset_(params.offset),
      device_(params.loc,
              std::make_unique<ecclesia::SysPciRegion>(params.loc)) {}

absl::optional<int> PciThermalSensor::Read() {
  uint16_t t;
  absl::Status status = device_.config_space().region()->Read16(offset_, &t);
  if (status.ok()) {
    return t;
  }
  return absl::nullopt;
}

std::vector<PciThermalSensor> CreatePciThermalSensors(
    const absl::Span<const PciSensorParams> param_set) {
  std::vector<PciThermalSensor> sensors;
  for (const auto &param : param_set) {
    sensors.emplace_back(param);
  }
  return sensors;
}

}  // namespace ecclesia
