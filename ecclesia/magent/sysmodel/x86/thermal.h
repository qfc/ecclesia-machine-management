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

#ifndef ECCLESIA_MAGENT_X86_THERMAL_H_
#define ECCLESIA_MAGENT_X86_THERMAL_H_

#include <cstdint>
#include <string>
#include <vector>

#include "ecclesia/magent/sysmodel/thermal.h"
#include "ecclesia/magent/lib/io/pci.h"

namespace ecclesia {

// PCI thermal device

// Parameters to construct PciThermalSensor. Note that the actual name string
// object should outlive the params object because of the string_view.
struct PciSensorParams {
  absl::string_view name;
  ecclesia::PciLocation loc;
  // Sensor reading offset.
  size_t offset;
  int upper_threshold_critical;
};

class PciThermalSensor : public ThermalSensor {
 public:
  explicit PciThermalSensor(const PciSensorParams &params);
  virtual ~PciThermalSensor() = default;

  // Disable copy, since PciDevice is not copyable.
  PciThermalSensor(const PciThermalSensor&) = delete;
  PciThermalSensor &operator=(const PciThermalSensor&) = delete;

  // But enable move.
  PciThermalSensor(PciThermalSensor&&) = default;
  PciThermalSensor &operator=(PciThermalSensor&&) = default;

  absl::optional<int> Read() override;

 private:
  // Thermal info offset
  const size_t offset_;
  PciDevice device_;
};

std::vector<PciThermalSensor> CreatePciThermalSensors(
    const absl::Span<const PciSensorParams> param_set);

// CPU thermal

// Note that the actual name string object should outlive the params object
// because of the string_view.
struct CpuMarginSensorParams {
  absl::string_view name;
  int cpu_index;
};

class CpuMarginSensor : public ThermalSensor {
 public:
  explicit CpuMarginSensor(const CpuMarginSensorParams &params);
  virtual ~CpuMarginSensor() = default;

  absl::optional<int> Read() override;

 private:
  absl::optional<std::string> lpu_path_;
};

std::vector<CpuMarginSensor> CreateCpuMarginSensors(
    const absl::Span<const CpuMarginSensorParams> param_set);

}  // namespace ecclesia
#endif  // ECCLESIA_MAGENT_X86_THERMAL_H_
