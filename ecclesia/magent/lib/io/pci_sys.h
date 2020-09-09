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
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ecclesia/lib/apifs/apifs.h"
#include "ecclesia/magent/lib/io/pci.h"
#include "ecclesia/magent/lib/io/pci_location.h"

namespace ecclesia {

class SysPciRegion : public PciRegion {
 public:
  explicit SysPciRegion(const PciLocation &pci_loc);

  // This constructor allows customized sysfs PCI devices directory, mostly for
  // testing purpose.
  SysPciRegion(std::string sys_pci_devices_dir, const PciLocation &pci_loc);

  absl::StatusOr<uint8_t> Read8(size_t offset) const override;
  absl::Status Write8(size_t offset, uint8_t data) override;

  absl::StatusOr<uint16_t> Read16(size_t offset) const override;
  absl::Status Write16(size_t offset, uint16_t data) override;

  absl::StatusOr<uint32_t> Read32(size_t offset) const override;
  absl::Status Write32(size_t offset, uint32_t data) override;

  absl::Status ReadBytes(uint64_t offset,
                         absl::Span<char> value) const override;
  absl::Status WriteBytes(uint64_t offset,
                          absl::Span<const char> value) override;

 private:
  std::string sys_pci_devices_dir_;
  PciLocation loc_;
  ApifsFile apifs_;
};

class SysfsPciResources : public PciResources {
 public:
  explicit SysfsPciResources(PciLocation loc);
  SysfsPciResources(std::string sys_pci_devices_dir, PciLocation loc);

  bool Exists() override;

 private:
  // Resource flag that indicate that a resource is an I/O resource.
  static constexpr uint64_t kIoResourceFlag = 0x100;

  absl::StatusOr<BarInfo> GetBaseAddressImpl(BarNum bar_id) const override;

  ApifsDirectory apifs_;
};

class SysfsPciDevice : public PciDevice {
 public:
  explicit SysfsPciDevice(const PciLocation &location)
      : PciDevice(location, std::make_unique<SysPciRegion>(location),
                  std::make_unique<SysfsPciResources>(location)) {}
};

class SysfsPciDiscovery : public PciDiscoveryInterface {
 public:
  SysfsPciDiscovery();

  // This constructor allows customized sysfs PCI devices directory, mostly for
  // testing purpose.
  SysfsPciDiscovery(const std::string &sys_pci_devices_dir);

  absl::StatusOr<std::vector<PciLocation>> EnumerateAllDevices() const override;

 private:
  std::string sys_pci_devices_dir_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_SYS_H_
