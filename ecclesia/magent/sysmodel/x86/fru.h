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

#ifndef ECCLESIA_MAGENT_SYSMODEL_X86_FRU_H_
#define ECCLESIA_MAGENT_SYSMODEL_X86_FRU_H_

#include <memory>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/magent/lib/eeprom/smbus_eeprom.h"

namespace ecclesia {

struct FruInfo {
  std::string product_name;
  std::string manufacturer;
  std::string serial_number;
  std::string part_number;
};

class SysmodelFru {
 public:
  SysmodelFru(FruInfo fru_info);

  // Allow the object to be copyable
  // Make sure that copy construction is relatively light weight.
  // In cases where it is not feasible to copy construct data members,it may
  // make sense to wrap the data member in a shared_ptr.
  SysmodelFru(const SysmodelFru &dimm) = default;
  SysmodelFru &operator=(const SysmodelFru &dimm) = default;

  absl::string_view GetManufacturer() const;
  absl::string_view GetSerialNumber() const;
  absl::string_view GetPartNumber() const;

 private:
  FruInfo fru_info_;
};

// SysmodelFruReader provides a caching interface for reading FRUs. If it is
// successful in reading the FRU, it will return a copy of that successful
// read for the rest of its lifetime.
class SysmodelFruReader {
 public:
  SysmodelFruReader(SmbusEeprom2ByteAddr::Option option)
      : option_(std::move(option)) {}

  // Returns the FRU contents if available.
  // If the FRU contents are cached, the cached content is returned. Otherwise
  // performs the low level FRU read, and if successful, populates the cache
  // and returns the read.
  absl::optional<SysmodelFru> Read();

  // Since we're holding caches, we should not allow copies.
  SysmodelFruReader(const SysmodelFruReader &dimm) = delete;
  SysmodelFruReader &operator=(const SysmodelFruReader &dimm) = delete;

 private:
  SmbusEeprom2ByteAddr::Option option_;
  // Stores the cached FRU that was read.
  absl::optional<SysmodelFru> cached_fru_;
};

absl::flat_hash_map<std::string, std::unique_ptr<SysmodelFruReader>> CreateFrus(
    absl::Span<SmbusEeprom2ByteAddr::Option> options);

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_SYSMODEL_X86_FRU_H_
