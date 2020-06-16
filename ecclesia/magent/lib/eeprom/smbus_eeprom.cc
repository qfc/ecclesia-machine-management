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

#include "ecclesia/magent/lib/eeprom/smbus_eeprom.h"

#include <string.h>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>

#include "absl/status/status.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/magent/lib/io/smbus.h"

namespace ecclesia {

SmbusEeprom2ByteAddr::SmbusEeprom2ByteAddr(Option option)
    : option_(std::move(option)) {}

absl::optional<int> SmbusEeprom2ByteAddr::SequentialRead(
    size_t offset, absl::Span<unsigned char> value) const {
  // We can't actually use smbus block read because the driver doesn't know how
  // to do the 2-byte address write. So we do the best we can by performing the
  // address write once, then calling read byte repeatedly to keep the overhead
  // to a minimum.

  // Write the eeprom offset as a command byte + data byte.
  uint8_t hi = offset >> 8;
  uint8_t lo = offset & 0xff;
  size_t len = value.size();

  absl::Status status = option_.device.Write8(hi, lo);
  if (!status.ok()) {
    std::cout << "smbus device " << option_.device.location()
              << " Failed to write smbus register 0x" << std::hex << offset
              << '\n';
    return absl::nullopt;
  }

  // Read the block a byte a time.
  int i = 0;
  for (i = 0; i < len; ++i) {
    uint8_t val;
    if (!option_.device.ReceiveByte(&val).ok()) {
      std::cout << "smbus device " << option_.device.location()
                << " Failed to read smbus register 0x" << std::hex
                << offset + i;
      return absl::nullopt;
    }
    value[i] = val;
  }

  return i;
}

absl::optional<int> SmbusEeprom2ByteAddr::ReadBytes(
    size_t offset, absl::Span<unsigned char> value) const {
  memset(value.data(), 0, value.size());

  absl::MutexLock ml(&device_mutex_);

  return SequentialRead(offset, value);
}

absl::optional<int> SmbusEeprom2ByteAddr::WriteBytes(
    size_t offset, absl::Span<const unsigned char> data) const {
  return absl::nullopt;
}

}  // namespace ecclesia
