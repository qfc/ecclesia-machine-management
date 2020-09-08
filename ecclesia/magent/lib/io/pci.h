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

class PciResources {
 public:
  // Type of address used in a BAR.
  enum BarType { kBarTypeMem, kBarTypeIo };

  // An identifier representing a BAR ID.
  class BarNum : public FixedRangeInteger<BarNum, int, 0, 5> {
   public:
    explicit constexpr BarNum(BaseType value) : BaseType(value) {}
  };

  explicit PciResources(PciLocation loc) : loc_(std::move(loc)) {}
  virtual ~PciResources() = default;

  // Check if Pci exists.
  virtual bool Exists() = 0;

  // Get information about a BAR.
  struct BarInfo {
    BarType type;
    uint64_t address;
  };
  absl::StatusOr<BarInfo> GetBaseAddress(BarNum bar_id) const {
    return GetBaseAddressImpl(bar_id);
  }
  // Templated version for when the BAR number is known at compile time.
  template <int BarId>
  absl::StatusOr<BarInfo> GetBaseAddress() const {
    return GetBaseAddressImpl(BarNum::Make<BarId>());
  }

 protected:
  PciLocation loc_;

 private:
  // The underlying implementation of GetBaseAddress.
  virtual absl::StatusOr<BarInfo> GetBaseAddressImpl(BarNum bar_id) const = 0;
};

// A wrappper class for interacting with a PCI device.
class PciDevice {
 public:
  // Create a PCI device using the provided region for all config space access.
  PciDevice(const PciLocation &location,
            std::unique_ptr<PciRegion> config_region,
            std::unique_ptr<PciResources> resources_intf)
      : location_(location),
        config_region_(std::move(config_region)),
        config_space_(config_region_.get()),
        resources_intf_(std::move(resources_intf)) {}

  PciDevice(const PciDevice &) = delete;
  PciDevice &operator=(const PciDevice &) = delete;

  PciDevice(PciDevice &&) = default;
  PciDevice &operator=(PciDevice &&) = default;

  // Get this device address.
  const PciLocation &Location() const { return location_; }

  // Get the device config space.
  PciConfigSpace &ConfigSpace() { return config_space_; }
  const PciConfigSpace &ConfigSpace() const { return config_space_; }

  // Get the device resource information.
  PciResources &Resources() { return *resources_intf_; }
  const PciResources &Resources() const { return *resources_intf_; }

 private:
  PciLocation location_;

  std::unique_ptr<PciRegion> config_region_;
  PciConfigSpace config_space_;

  std::unique_ptr<PciResources> resources_intf_;
};

// This class represents a node in the PCI topology tree.
class PciTopologyNode {
 public:
  PciTopologyNode(const PciLocation &location, size_t depth,
                  PciTopologyNode *parent)
      : location_(location), depth_(depth), parent_(parent) {}

  PciTopologyNode(const PciTopologyNode &) = delete;
  PciTopologyNode &operator=(const PciTopologyNode &) = delete;
  PciTopologyNode(PciTopologyNode &&) = default;

  const PciLocation &Location() const { return location_; }

  // This gets the depth in the PCI tree. For a root node, the depth is 0.
  size_t Depth() const { return depth_; }

  // This gets the parent of this PciTopologyNode, for a root node, this
  // function returns nullptr.
  PciTopologyNode *Parent() const { return parent_; }

  // This gets the children of this PciTopologyNode, for a endpoint node without
  // children, the return vector is empty.
  std::vector<PciTopologyNode *> Children() const { return children_; }

 private:
  friend class SysfsPciDiscovery;
  void AddChild(PciTopologyNode *node) { children_.push_back(node); }

  void SetChildren(std::vector<PciTopologyNode *> children) {
    children_ = std::move(children);
  }

  const PciLocation location_;
  size_t depth_;
  PciTopologyNode *parent_;
  std::vector<PciTopologyNode *> children_;
};

// This interface class defines some methods for discovering the PCI locations
// and topologies.
class PciDiscoveryInterface {
 public:
  virtual ~PciDiscoveryInterface() = default;

  virtual std::vector<PciLocation> EnumerateAllLocations() const = 0;

  // This function returns a vector of root PciTopologyNodes whose depths are 0.
  virtual std::vector<PciTopologyNode *> GetRootNodes() const = 0;

  // This function returns a PciTopologyNode given a PCI location, or a status
  // if there is no such node found.
  virtual absl::StatusOr<PciTopologyNode *> GetNode(
      const PciLocation &location) const = 0;

  // This function returns the ancestor (root node) of the given PCI location.
  // Or if there is no such node found, it returns a status.
  virtual absl::StatusOr<PciTopologyNode *> GetAncestorNode(
      const PciLocation &location) const {
    auto maybe_node = GetNode(location);
    if (!maybe_node.ok()) {
      return maybe_node.status();
    }
    PciTopologyNode *node = maybe_node.value();
    while (node && node->Parent()) {
      node = node->Parent();
    }
    return node;
  }

  virtual absl::Status Rescan() = 0;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_H_
