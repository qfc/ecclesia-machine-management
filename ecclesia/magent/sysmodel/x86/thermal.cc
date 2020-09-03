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
#include <memory>
#include <string>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/lib/io/constants.h"
#include "ecclesia/lib/io/msr.h"
#include "ecclesia/magent/lib/event_logger/intel_cpu_topology.h"
#include "ecclesia/magent/lib/io/pci.h"
#include "ecclesia/magent/lib/io/pci_sys.h"
#include "ecclesia/magent/sysmodel/thermal.h"

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

CpuMarginSensor::CpuMarginSensor(const CpuMarginSensorParams &params)
    // Right now we don’t know the upper critical limit for (at least some
    // Intel) CPUs. So it is set to some arbitrary number.
    : ThermalSensor(params.name, 0), lpu_path_(absl::nullopt) {
  // Determine the LPU index to use.
  IntelCpuTopology top;
  std::vector<int> lpus = top.GetLpusForSocketId(params.cpu_index);
  if (!lpus.empty()) {
    lpu_path_ = absl::StrCat("/dev/cpu/", lpus[0], "/msr");
  }
}

absl::optional<int> CpuMarginSensor::Read() {
  if (!lpu_path_) {
    return absl::nullopt;
  }

  Msr msr(*lpu_path_);
  absl::StatusOr<uint64_t> maybe_therm_status =
      msr.Read(kMsrIa32PackageThermStatus);
  if (!maybe_therm_status.ok()) {
    return absl::nullopt;
  }

  // The following code is in gsys, but I don’t think this works for package
  // thermal. In Intel’s manual, IA32_PACKAGE_THERM_STATUS’s bit 31 is said to
  // be “reserved”.
  //
  // Bit 31 indicates valid reading.
  // if (!(val & (1 << 31))) { return absl::nullopt; }

  // Readout is in bits 22:16.
  uint64_t reading = (*maybe_therm_status >> 16) & 0x7f;
  return reading;
}

std::vector<CpuMarginSensor> CreateCpuMarginSensors(
    const absl::Span<const CpuMarginSensorParams> param_set) {
  std::vector<CpuMarginSensor> sensors;
  for (const auto &param : param_set) {
    sensors.emplace_back(param);
  }
  return sensors;
}

}  // namespace ecclesia
