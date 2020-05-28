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

// Provide basic operations for interacting with PCI devices. This library
// provides several abstractions at different layers:
//   * PciLocation, a value object representing a PCI BDF-style address
//   * PciRegion, a low-level interface accessing a PCI address space
//   * PciConfigSpace and PciDevice, objects that provide higher-level
//     operations built on top of PciRegion

#ifndef ECCLESIA_MAGENT_LIB_IO_PCI_H_
#define ECCLESIA_MAGENT_LIB_IO_PCI_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "absl/status/status.h"
#include "magent/lib/io/pci_location.h"
#include "magent/lib/io/pci_regs.h"

namespace ecclesia {

// PciRegion represents an abstract PCI address space, for example config space
// or a BAR space. It has a fixed size and allows you to do fixed-size read and
// write operations into it.
class PciRegion {
 public:
  explicit PciRegion(size_t size) : size_(size) {}
  virtual ~PciRegion() = default;

  // Size of this region.
  size_t size() const { return size_; }

  // Read/write function that access within this region
  // Offset is relative to the start of the region.
  virtual absl::Status Read8(size_t offset, uint8_t *data) = 0;
  virtual absl::Status Write8(size_t offset, uint8_t data) = 0;

  virtual absl::Status Read16(size_t offset, uint16_t *data) = 0;
  virtual absl::Status Write16(size_t offset, uint16_t data) = 0;

  virtual absl::Status Read32(size_t offset, uint32_t *data) = 0;
  virtual absl::Status Write32(size_t offset, uint32_t data) = 0;

 private:
  const size_t size_;
};

// An object for reading and writing information from config space. It
// requires a low-level interface for actually doing raw reads and writes.
class PciConfigSpace {
 public:
  explicit PciConfigSpace(PciRegion *region) : region_(region) {}

  absl::Status vendor_id(uint16_t *vendor_id) const {
    return region_->Read16(kPciVidReg, vendor_id);
  }

  absl::Status device_id(uint16_t *device_id) const {
    return region_->Read16(kPciDidReg, device_id);
  }

  absl::Status command(uint16_t *command) const {
    return region_->Read16(kPciCommandReg, command);
  }

  absl::Status status(uint16_t *status) const {
    return region_->Read16(kPciStatusReg, status);
  }

  absl::Status revision_id(uint8_t *revision_id) const {
    return region_->Read8(kPciRevisionIdReg, revision_id);
  }

  absl::Status class_code(uint16_t *class_code) const {
    return region_->Read16(kPciClassCodeReg, class_code);
  }

  absl::Status subsystem_vendor_id(uint16_t *subsystem_vendor_id) const {
    return region_->Read16(kPciType0SubsysVendorIdReg, subsystem_vendor_id);
  }

  absl::Status subsystem_id(uint16_t *subsystem_id) const {
    return region_->Read16(kPciType0SubsysIdReg, subsystem_id);
  }

  absl::Status capacity_pointer(uint16_t *capacity_pointer) const {
    return region_->Read16(kPciCapPointerReg, capacity_pointer);
  }

  absl::Status write_command(uint16_t value) {
    return region_->Write16(kPciCommandReg, value);
  }

 private:
  PciRegion *region_;
};

// An object for interacting with a PCI device.
class PciDevice {
 public:
  // Create a PCI device using the provided region for all config space access.
  PciDevice(const PciLocation &location,
            std::unique_ptr<PciRegion> config_region)
      : location_(location),
        config_region_(std::move(config_region)),
        config_(config_region_.get()) {}

  PciDevice(const PciDevice &) = delete;
  PciDevice &operator=(const PciDevice &) = delete;

  // Get this device address.
  const PciLocation &location() const { return location_; }

  // Get the device config space.
  PciConfigSpace &config_space() { return config_; }
  const PciConfigSpace &config_space() const { return config_; }

 private:
  PciLocation location_;

  std::unique_ptr<PciRegion> config_region_;
  PciConfigSpace config_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_H_
