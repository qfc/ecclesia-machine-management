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

#include "ecclesia/magent/sysmodel/x86/sysmodel.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/optional.h"
#include "ecclesia/lib/smbios/reader.h"
#include "ecclesia/lib/time/clock.h"
#include "ecclesia/magent/lib/event_logger/event_logger.h"
#include "ecclesia/magent/lib/event_reader/elog_reader.h"
#include "ecclesia/magent/lib/event_reader/event_reader.h"
#include "ecclesia/magent/lib/event_reader/mced_reader.h"
#include "ecclesia/magent/sysmodel/x86/cpu.h"
#include "ecclesia/magent/sysmodel/x86/dimm.h"
#include "ecclesia/magent/sysmodel/x86/fru.h"
#include "ecclesia/magent/sysmodel/x86/thermal.h"

namespace ecclesia {

std::size_t SystemModel::NumDimms() const {
  absl::ReaderMutexLock ml(&dimms_lock_);
  return dimms_.size();
}

absl::optional<Dimm> SystemModel::GetDimm(std::size_t index) {
  absl::ReaderMutexLock ml(&dimms_lock_);
  if (index < dimms_.size()) {
    return dimms_[index];
  }
  return absl::nullopt;
}

std::size_t SystemModel::NumDimmThermalSensors() const {
  absl::ReaderMutexLock ml(&dimm_thermal_sensors_lock_);
  return dimm_thermal_sensors_.size();
}

PciThermalSensor *SystemModel::GetDimmThermalSensor(std::size_t index) {
  absl::ReaderMutexLock ml(&dimm_thermal_sensors_lock_);
  if (index < dimm_thermal_sensors_.size()) {
    return &dimm_thermal_sensors_[index];
  }
  return nullptr;
}

std::size_t SystemModel::NumCpus() const {
  absl::ReaderMutexLock ml(&cpus_lock_);
  return cpus_.size();
}

absl::optional<Cpu> SystemModel::GetCpu(std::size_t index) {
  absl::ReaderMutexLock ml(&cpus_lock_);
  if (index < cpus_.size()) {
    return cpus_[index];
  }
  return absl::nullopt;
}

std::size_t SystemModel::NumCpuMarginSensors() const {
  absl::ReaderMutexLock ml(&cpu_margin_sensors_lock_);
  return cpu_margin_sensors_.size();
}

absl::optional<CpuMarginSensor> SystemModel::GetCpuMarginSensor(
    std::size_t index) {
  absl::ReaderMutexLock ml(&cpu_margin_sensors_lock_);
  if (index < cpu_margin_sensors_.size()) {
    return cpu_margin_sensors_[index];
  }
  return absl::nullopt;
}

std::size_t SystemModel::NumFruReaders() const {
  absl::ReaderMutexLock ml(&fru_readers_lock_);
  return fru_readers_.size();
}

SysmodelFruReader *SystemModel::GetFruReader(absl::string_view fru_name) const {
  absl::ReaderMutexLock ml(&fru_readers_lock_);
  auto fru = fru_readers_.find(fru_name);
  if (fru != fru_readers_.end()) {
    return fru->second.get();
  }

  return nullptr;
}

SystemModel::SystemModel(SysmodelParams params)
    : smbios_reader_(absl::make_unique<SmbiosReader>(
          params.smbios_entry_point_path, params.smbios_tables_path)),
      field_translator_(std::move(params.field_translator)),
      dimm_thermal_params_(std::move(params.dimm_thermal_params)),
      cpu_margin_params_(std::move(params.cpu_margin_params)) {
  // Construct system model objects
  auto dimms = CreateDimms(smbios_reader_.get(), field_translator_.get());
  {
    absl::WriterMutexLock ml(&dimms_lock_);
    dimms_ = std::move(dimms);
  }

  auto cpus = CreateCpus(*smbios_reader_);
  {
    absl::WriterMutexLock ml(&cpus_lock_);
    cpus_ = std::move(cpus);
  }

  auto fru_readers = CreateFrus(params.fru_factories);
  {
    absl::WriterMutexLock ml(&fru_readers_lock_);
    fru_readers_ = std::move(fru_readers);
  }

  auto dimm_thermal_sensors = CreatePciThermalSensors(dimm_thermal_params_);
  {
    absl::WriterMutexLock ml(&dimm_thermal_sensors_lock_);
    dimm_thermal_sensors_ = std::move(dimm_thermal_sensors);
  }

  auto cpu_margin_sensors = CreateCpuMarginSensors(cpu_margin_params_);
  {
    absl::WriterMutexLock ml(&cpu_margin_sensors_lock_);
    cpu_margin_sensors_ = std::move(cpu_margin_sensors);
  }

  // Create event readers to feed into the event logger
  std::vector<std::unique_ptr<SystemEventReader>> readers;
  readers.push_back(absl::make_unique<McedaemonReader>(params.mced_socket_path,
                                                       &mcedaemon_socket_));
  if (auto system_event_log = smbios_reader_->GetSystemEventLog()) {
    readers.push_back(absl::make_unique<ElogReader>(
        std::move(system_event_log), params.sysfs_mem_file_path));
  }

  event_logger_ = absl::make_unique<SystemEventLogger>(std::move(readers),
                                                       Clock::RealClock());
}

}  // namespace ecclesia
