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
// Basic read/write routines for SMBus devices.

#ifndef ECCLESIA_MAGENT_LIB_IO_SMBUS_H_
#define ECCLESIA_MAGENT_LIB_IO_SMBUS_H_

#include <stddef.h>

#include <string>
#include <utility>

#include "absl/base/integral_types.h"
#include "absl/status/status.h"
#include "absl/types/span.h"

namespace ecclesia {

// Bus location information for an I2C/SMBus bus. A bus can be identified with
// just a number so we just use a typedef instead of an explicit class.
struct SmbusBus {
  int loc;
};

struct SmbusAddress {
  int addr;
};

// Device location information for an I2C/SMBus device.  It's POD-like,
// but the methods and constructor make it friendlier to use. Because it
// is not *really* POD, it can not be used as a static instance.
struct SmbusLocation {
  // Constants for bounds checking.
  static constexpr int kBusMax = 256;
  static constexpr int kAddressMax = 128;

  // Number of the bus the device is attached to. [0-255]
  SmbusBus bus;

  // 7-bit address of the device on the bus. [0-127]
  SmbusAddress address;

  // Support hashing of locations for use as a key in hash maps.
  template <typename H>
  friend H AbslHashValue(H h, const SmbusLocation &loc) {
    return H::combine(std::move(h), loc.bus, loc.address);
  }
};

std::ostream &operator<<(std::ostream &os, const SmbusLocation &location);

// SmbusLocation relational operators.
// Order is equivalent to that of a <bus, address> tuple.
bool operator==(const SmbusLocation &lhs, const SmbusLocation &rhs);
inline bool operator!=(const SmbusLocation &lhs, const SmbusLocation &rhs) {
  return !(lhs == rhs);
}
bool operator<(const SmbusLocation &lhs, const SmbusLocation &rhs);
inline bool operator>(const SmbusLocation &lhs, const SmbusLocation &rhs) {
  return rhs < lhs;
}
inline bool operator<=(const SmbusLocation &lhs, const SmbusLocation &rhs) {
  return !(rhs < lhs);
}
inline bool operator>=(const SmbusLocation &lhs, const SmbusLocation &rhs) {
  return !(lhs < rhs);
}

// An interface to access SMBus devices.
class SmbusAccessInterface {
 public:
  SmbusAccessInterface() {}
  virtual ~SmbusAccessInterface() = default;

  // Probe for presence of a device at the requested location.
  // NOTE: This is *NOT* guaranteed to detect all types of devices.
  virtual absl::Status ProbeDevice(const SmbusLocation &loc) const = 0;

  // Sends/Receives a byte of data to/from a SMBus device.
  virtual absl::Status WriteQuick(const SmbusLocation &loc,
                                  uint8_t data) const = 0;
  virtual absl::Status SendByte(const SmbusLocation &loc,
                                uint8_t data) const = 0;
  virtual absl::Status ReceiveByte(const SmbusLocation &loc,
                                   uint8_t *data) const = 0;

  // Reads/Writes data to a SMBus device specifying the command code
  // (or register location).
  virtual absl::Status Write8(const SmbusLocation &loc, int command,
                              uint8_t data) const = 0;
  virtual absl::Status Read8(const SmbusLocation &loc, int command,
                             uint8_t *data) const = 0;
  virtual absl::Status Write16(const SmbusLocation &loc, int command,
                               uint16_t data) const = 0;
  virtual absl::Status Read16(const SmbusLocation &loc, int command,
                              uint16_t *data) const = 0;

  // Reads/Writes a block of data to a SMBus device specifying the
  // command code (or register location).
  // Args:
  //   command: command code or register location
  //   data: buffer to put/receive data
  //   len: output parameter for number of bytes read
  virtual absl::Status WriteBlock(
      const SmbusLocation &loc, int command,
      absl::Span<const unsigned char> data) const = 0;
  virtual absl::Status ReadBlock(const SmbusLocation &loc, int command,
                                 absl::Span<unsigned char> data,
                                 size_t *len) const = 0;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_SMBUS_H_
