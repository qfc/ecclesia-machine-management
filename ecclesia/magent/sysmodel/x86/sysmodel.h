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

#ifndef ECCLESIA_MAGENT_SYSMODEL_X86_SYSMODEL_H_
#define ECCLESIA_MAGENT_SYSMODEL_X86_SYSMODEL_H_

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/lib/smbios/platform_translator.h"
#include "ecclesia/lib/smbios/reader.h"
#include "ecclesia/magent/lib/eeprom/smbus_eeprom.h"
#include "ecclesia/magent/lib/event_logger/event_logger.h"
#include "ecclesia/magent/lib/event_reader/mced_reader.h"
#include "ecclesia/magent/sysmodel/x86/cpu.h"
#include "ecclesia/magent/sysmodel/x86/dimm.h"
#include "ecclesia/magent/sysmodel/x86/fru.h"
#include "ecclesia/magent/sysmodel/x86/thermal.h"

namespace ecclesia {

// Parameters required to construct the system model
struct SysmodelParams {
  std::unique_ptr<SmbiosFieldTranslator> field_translator;
  std::string smbios_entry_point_path;
  std::string smbios_tables_path;
  std::string mced_socket_path;
  std::string sysfs_mem_file_path;
  absl::Span<SmbusEeprom2ByteAddr::Option> eeprom_options;
  absl::Span<const PciSensorParams> dimm_thermal_params;
  absl::Span<const CpuMarginSensorParams> cpu_margin_params;
};

// The SystemModel must be thread safe
class SystemModel {
 public:
  explicit SystemModel(SysmodelParams params);
  ~SystemModel() {}

  std::size_t NumDimms() const;
  absl::optional<Dimm> GetDimm(std::size_t index);

  // The number of DIMM thermal sensors. This should be the same as the number
  // of DIMMs.
  std::size_t NumDimmThermalSensors() const;
  // Return the sensor for DIMM at “index”. Return nullptr if index is out of
  // bounds.
  PciThermalSensor *GetDimmThermalSensor(std::size_t index);

  std::size_t NumCpus() const;
  absl::optional<Cpu> GetCpu(std::size_t index);

  std::size_t NumCpuMarginSensors() const;
  absl::optional<CpuMarginSensor> GetCpuMarginSensor(std::size_t index);

  std::size_t NumFrus() const;
  template <typename IteratorF>
  void GetFrus(IteratorF iterator) const {
    absl::ReaderMutexLock ml(&frus_lock_);
    for (const auto &f : frus_) {
      iterator(f.first, f.second);
    }
  }
  absl::optional<SysmodelFru> GetFru(absl::string_view fru_name) const;

  // The event logger logs all of the system events with respect to cpu and dimm
  // errors. This method provides a mechanism to process the events for error
  // reporting.
  void VisitSystemEvents(SystemEventVisitor *visitor) {
    if (event_logger_) {
      event_logger_->Visit(visitor);
    }
  }

 private:
  // Platform interfaces
  std::unique_ptr<SmbiosReader> smbios_reader_;
  std::unique_ptr<SmbiosFieldTranslator> field_translator_;
  LibcMcedaemonSocket mcedaemon_socket_;

  // System model objects

  mutable absl::Mutex dimms_lock_;
  std::vector<Dimm> dimms_ ABSL_GUARDED_BY(dimms_lock_);

  mutable absl::Mutex cpus_lock_;
  std::vector<Cpu> cpus_ ABSL_GUARDED_BY(cpus_lock_);

  mutable absl::Mutex frus_lock_;
  absl::flat_hash_map<std::string, SysmodelFru> frus_
      ABSL_GUARDED_BY(frus_lock_);

  mutable absl::Mutex dimm_thermal_sensors_lock_;
  std::vector<PciThermalSensor> dimm_thermal_sensors_
      ABSL_GUARDED_BY(dimm_thermal_sensors_lock_);

  mutable absl::Mutex cpu_margin_sensors_lock_;
  std::vector<CpuMarginSensor> cpu_margin_sensors_
      ABSL_GUARDED_BY(cpu_margin_sensors_lock_);

  std::unique_ptr<SystemEventLogger> event_logger_;

  absl::Span<SmbusEeprom2ByteAddr::Option> eeprom_options_;
  const absl::Span<const PciSensorParams> dimm_thermal_params_;
  const absl::Span<const CpuMarginSensorParams> cpu_margin_params_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_SYSMODEL_X86_SYSMODEL_H_
