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

// An interface for discovering USB devices.

#ifndef ECCLESIA_MAGENT_LIB_IO_USB_H_
#define ECCLESIA_MAGENT_LIB_IO_USB_H_

#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ecclesia/lib/types/fixed_range_int.h"

namespace ecclesia {
// Device signature information for a USB device.
struct UsbSignature {
  // Manufacturer of the device.
  uint16_t vendor_id;
  // Product identifier as allocated by the manufacturer.
  uint16_t product_id;
};

// The port number a USB device is connected to.
class UsbPort : public FixedRangeInteger<UsbPort, int, 1, 15> {
 public:
  explicit constexpr UsbPort(BaseType value) : BaseType(value) {}
};

// Class that describes the sequence of ports a USB device is connected to.
// Each device attached downstream of a USB root controller can be uniquely
// identified by the sequence of port numbers it's attached through.
//
// In the USB standard a single hub (or root controller) can have up to 15
// downstream ports, numbered 1-15. An endpoint device can have as many as six
// hubs/controllers upstream from it. Thus, we represent the sequence of ports
// as an array of 0-6 integers.
class UsbPortSequence {
 public:
  // The maximum length of a chain of USB devices (through hubs).
  static constexpr int kDeviceChainMaxLength = 6;

  // constructor for empty port sequence.
  UsbPortSequence() = default;
  static absl::optional<UsbPortSequence> TryMake(absl::Span<const int> ports);

  UsbPortSequence(const UsbPortSequence &other) = default;
  UsbPortSequence &operator=(const UsbPortSequence &other) = default;

  size_t Size() const;
  absl::optional<UsbPort> Port(size_t index) const;

  absl::optional<UsbPortSequence> Downstream(UsbPort port) const;

  friend bool operator==(const UsbPortSequence &lhs,
                         const UsbPortSequence &rhs);
  friend bool operator!=(const UsbPortSequence &lhs,
                         const UsbPortSequence &rhs);

 private:
  UsbPortSequence(absl::Span<const UsbPort> ports);
  std::vector<UsbPort> ports_;
};

// This is not a hard limit from any specification, but we're unlikely to
// encounter a system with more than a couple hundred USB host controllers.
class UsbBusLocation : public FixedRangeInteger<UsbBusLocation, int, 1, 255> {
 public:
  explicit constexpr UsbBusLocation(BaseType value) : BaseType(value) {}
};

// Device location information for a USB device. This class has value semantics.
// We can uniquely represent a USB device attached to a system by the bus
// number it lives on plus the sequence of ports you follow to reach it from
// the root controller.
class UsbLocation {
 public:
  UsbLocation(UsbBusLocation bus, const UsbPortSequence &ports)
      : bus_(bus), ports_(ports) {}

  // Read the bus and port numbers.
  UsbBusLocation Bus() const { return bus_; }
  size_t NumPorts() const { return ports_.Size(); }
  absl::optional<UsbPort> Port(size_t index) const {
    return ports_.Port(index);
  }

  friend bool operator==(const UsbLocation &lhs, const UsbLocation &rhs);
  friend bool operator!=(const UsbLocation &lhs, const UsbLocation &rhs);

 private:
  // The number of the bus behind a single controller. 1-255.
  UsbBusLocation bus_;
  // The sequence of ports that the device is connected behind.
  UsbPortSequence ports_;
};

// An interface to discover USB devices.
class UsbDiscoveryInterface {
 public:
  virtual ~UsbDiscoveryInterface() = default;

  // Enumerate all USB devices and push their location information into the
  // provided 'devices' vector.
  virtual absl::Status EnumerateAllUsbDevices(
      std::vector<UsbLocation> *devices) const = 0;
};

// An interface to access USB devices.
class UsbAccessInterface {
 public:
  UsbAccessInterface() = default;
  UsbAccessInterface(const UsbLocation &usb_location);
  virtual ~UsbAccessInterface() = default;

  // Gets the signature of the device at the given location.
  virtual absl::StatusOr<UsbSignature> GetSignature() const = 0;
};
}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_USB_H_
