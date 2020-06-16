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

// A class for reading SMBus compatible I2C EEPROMs.

#ifndef ECCLESIA_MAGENT_LIB_EEPROM_SMBUS_EEPROM_H_
#define ECCLESIA_MAGENT_LIB_EEPROM_SMBUS_EEPROM_H_

#include <cstddef>
#include <string>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/magent/lib/eeprom/eeprom.h"
#include "ecclesia/magent/lib/io/smbus.h"

namespace ecclesia {

// A class representing EEPROMs that range in size from 8k-bit to 512k-bit
// and are addressed using a 2-byte offset.
class SmbusEeprom2ByteAddr : public Eeprom {
 public:
  struct Option {
    std::string name;
    Eeprom::SizeType size;
    Eeprom::ModeType mode;
    SmbusDevice device;
  };

  explicit SmbusEeprom2ByteAddr(Option option);
  SmbusEeprom2ByteAddr(const SmbusEeprom2ByteAddr &) = delete;
  SmbusEeprom2ByteAddr &operator=(const SmbusEeprom2ByteAddr &) = delete;

  // Methods from Eeprom.
  Eeprom::SizeType GetSize() const override { return option_.size; }
  Eeprom::ModeType GetMode() const override { return option_.mode; }

  // Currently only support sequential read
  absl::optional<int> ReadBytes(size_t offset,
                                absl::Span<unsigned char> value) const override;
  absl::optional<int> WriteBytes(
      size_t offset, absl::Span<const unsigned char> data) const override;

 private:
  // Reads the eeprom one byte at a time.
  absl::optional<int> SequentialRead(size_t offset,
                                     absl::Span<unsigned char> value) const
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(device_mutex_);

  Option option_;
  mutable absl::Mutex device_mutex_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_EEPROM_SMBUS_EEPROM_H_
