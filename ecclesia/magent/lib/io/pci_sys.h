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

// A class for access PCI devices through sysfs

#ifndef ECCLESIA_MAGENT_LIB_IO_PCI_SYS_H_
#define ECCLESIA_MAGENT_LIB_IO_PCI_SYS_H_

#include <cstddef>
#include <cstdint>
#include <string>

#include "absl/status/status.h"
#include "ecclesia/lib/apifs/apifs.h"
#include "ecclesia/magent/lib/io/pci.h"
#include "ecclesia/magent/lib/io/pci_location.h"

namespace ecclesia {

class SysPciRegion : public PciRegion {
 public:
  explicit SysPciRegion(PciLocation pci_loc);
  SysPciRegion(std::string sys_root, PciLocation pci_loc);

  absl::Status Read8(size_t offset, uint8_t *data) override;
  absl::Status Write8(size_t offset, uint8_t data) override;

  absl::Status Read16(size_t offset, uint16_t *data) override;
  absl::Status Write16(size_t offset, uint16_t data) override;

  absl::Status Read32(size_t offset, uint32_t *data) override;
  absl::Status Write32(size_t offset, uint32_t data) override;

  absl::Status ReadBytes(uint64_t offset, absl::Span<char> value) override;
  absl::Status WriteBytes(uint64_t offset,
                          absl::Span<const char> value) override;

 private:
  std::string sys_root_;
  PciLocation loc_;
  ApifsFile apifs_;
};

class SysfsPci : PciFunction {
 public:
  explicit SysfsPci(PciLocation loc);
  SysfsPci(std::string sys_dir, PciLocation loc);

  bool Exists() override;
  absl::Status GetBaseAddress(BarNum bar_id,
                              PciFunction::BAR *out_bar) const override;

  const SysPciRegion &GetConfig() { return config_; }

 private:
  ApifsDirectory sys_dir_;
  SysPciRegion config_;
};

class SysfsPciDiscovery : PciDiscoveryInterface {
 public:
  SysfsPciDiscovery(std::string sys_dir);

  absl::Status EnumerateAllDevices(
      std::vector<PciLocation> *devices) const override;

 private:
  std::string sys_dir_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_SYS_H_
