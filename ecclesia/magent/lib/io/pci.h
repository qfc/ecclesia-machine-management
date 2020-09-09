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
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ecclesia/lib/types/fixed_range_int.h"
#include "ecclesia/magent/lib/io/pci_location.h"
#include "ecclesia/magent/lib/io/pci_regs.h"

namespace ecclesia {

// PciRegion represents an abstract PCI address space, for example config space
// or a BAR space. It has a fixed size and allows you to do fixed-size read and
// write operations into it.
class PciRegion {
 public:
  explicit PciRegion(size_t size) : size_(size) {}
  virtual ~PciRegion() = default;

  // Size of this region.
  size_t Size() const { return size_; }

  // Read/write function that access within this region
  // Offset is relative to the start of the region.
  virtual absl::StatusOr<uint8_t> Read8(size_t offset) const = 0;
  virtual absl::Status Write8(size_t offset, uint8_t data) = 0;

  virtual absl::StatusOr<uint16_t> Read16(size_t offset) const = 0;
  virtual absl::Status Write16(size_t offset, uint16_t data) = 0;

  virtual absl::StatusOr<uint32_t> Read32(size_t offset) const = 0;
  virtual absl::Status Write32(size_t offset, uint32_t data) = 0;

  virtual absl::Status ReadBytes(uint64_t offset,
                                 absl::Span<char> value) const = 0;
  virtual absl::Status WriteBytes(uint64_t offset,
                                  absl::Span<const char> value) = 0;

 private:
  const size_t size_;
};

// An object for reading and writing information from config space. It
// requires a low-level interface for actually doing raw reads and writes.
class PciConfigSpace {
 public:
  explicit PciConfigSpace(PciRegion *region) : region_(region) {}

  virtual ~PciConfigSpace() = default;

  virtual absl::StatusOr<uint16_t> VendorId() const {
    return region_->Read16(kPciVidReg);
  }

  virtual absl::StatusOr<uint16_t> DeviceId() const {
    return region_->Read16(kPciDidReg);
  }

  virtual absl::StatusOr<uint16_t> Command() const {
    return region_->Read16(kPciCommandReg);
  }

  virtual absl::StatusOr<uint16_t> Status() const {
    return region_->Read16(kPciStatusReg);
  }

  virtual absl::StatusOr<uint8_t> RevisionId() const {
    return region_->Read8(kPciRevisionIdReg);
  }

  // Class code occupies 3 bytes. So for a uint32_t return value, only the lower
  // 3 bytes are valid, the upper 1 byte is filled with 0.
  virtual absl::StatusOr<uint32_t> ClassCode() const {
    auto maybe_uint32 = region_->Read32(kPciClassCodeReg);
    if (!maybe_uint32.ok()) {
      return maybe_uint32.status();
    }
    return maybe_uint32.value() & 0x00ffffff;
  }

  virtual absl::StatusOr<uint16_t> SubsystemVendorId() const {
    return region_->Read16(kPciType0SubsysVendorIdReg);
  }

  virtual absl::StatusOr<uint16_t> subsystem_id() const {
    return region_->Read16(kPciType0SubsysIdReg);
  }

  virtual absl::StatusOr<uint16_t> CapacityPointer() const {
    return region_->Read16(kPciCapPointerReg);
  }

  virtual absl::Status WriteCommand(uint16_t value) {
    return region_->Write16(kPciCommandReg, value);
  }

  PciRegion *Region() { return region_; }

 private:
  PciRegion *region_;
};

// A wrappper class for interacting with a PCI device.
class PciDevice {
 public:
  // Create a PCI device using the provided region for all config space access.
  PciDevice(const PciLocation &location,
            std::unique_ptr<PciRegion> config_region)
      : location_(location),
        config_region_(std::move(config_region)),
        config_space_(config_region_.get()) {}

  PciDevice(const PciDevice &) = delete;
  PciDevice &operator=(const PciDevice &) = delete;

  PciDevice(PciDevice &&) = default;
  PciDevice &operator=(PciDevice &&) = default;

  // Get this device address.
  const PciLocation &Location() const { return location_; }

  // Get the device config space.
  PciConfigSpace &ConfigSpace() { return config_space_; }
  const PciConfigSpace &ConfigSpace() const { return config_space_; }

 private:
  PciLocation location_;

  std::unique_ptr<PciRegion> config_region_;
  PciConfigSpace config_space_;
};


class PciFunction {
 public:
  static constexpr uint8_t kMaxBars = 7;
  static constexpr uint64_t kIoResourceIo = 0x100ull;
  // Type of address used in a BAR.
  enum BarType { kBarTypeMem, kBarTypeIo };

  struct BAR {
    BarType type;
    uint64_t address;
  };

  // An identifier representing a BAR ID.
  class BarNum : public FixedRangeInteger<BarNum, int, 0, 5> {
   public:
    explicit constexpr BarNum(BaseType value) : BaseType(value) {}
  };

  explicit PciFunction(PciLocation loc) : loc_(loc) {}
  virtual ~PciFunction() = default;

  // Check if Pci exists.
  virtual bool Exists() = 0;

  // Get address of a BAR register.
  virtual absl::StatusOr<BAR> GetBaseAddress(BarNum bar_id) const = 0;

 protected:
  PciLocation loc_;
};

class PciDiscoveryInterface {
 public:
  virtual ~PciDiscoveryInterface() = default;

  virtual absl::StatusOr<std::vector<PciLocation>> EnumerateAllDevices()
      const = 0;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_H_
