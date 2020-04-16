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

#ifndef ECCLESIA_MAGENT_SYSMODEL_INTERFACE_H_
#define ECCLESIA_MAGENT_SYSMODEL_INTERFACE_H_

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/optional.h"
#include "magent/lib/event_logger/event_logger.h"
#include "magent/lib/smbios/platform_translator.h"
#include "magent/lib/smbios/reader.h"
#include "magent/sysmodel/cpu.h"
#include "magent/sysmodel/dimm.h"

namespace ecclesia {

// Parameters required to construct the system model
struct SysmodelParams {
  std::unique_ptr<SmbiosFieldTranslator> field_translator;
  std::string smbios_entry_point_path;
  std::string smbios_tables_path;
  std::string mced_socket_path;
  std::string sysfs_mem_file_path;
};

// The SystemModel must be thread safe
class SystemModel {
 public:
  explicit SystemModel(SysmodelParams params);
  ~SystemModel() {}

  std::size_t NumDimms() const;
  absl::optional<Dimm> GetDimm(std::size_t index);

  std::size_t NumCpus() const;
  absl::optional<Cpu> GetCpu(std::size_t index);

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
  // System model objects

  mutable absl::Mutex dimms_lock_;
  std::vector<Dimm> dimms_ ABSL_GUARDED_BY(dimms_lock_);

  mutable absl::Mutex cpus_lock_;
  std::vector<Cpu> cpus_ ABSL_GUARDED_BY(cpus_lock_);

  std::unique_ptr<SystemEventLogger> event_logger_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_SYSMODEL_INTERFACE_H_
