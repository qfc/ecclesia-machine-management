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

#include "magent/sysmodel/fru.h"

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"

namespace ecclesia {

Fru::Fru(FruInfo fru_info) : fru_info_(std::move(fru_info)) {}

absl::string_view Fru::GetManufacturer() const {
  return fru_info_.manufacturer;
}
absl::string_view Fru::GetSerialNumber() const {
  return fru_info_.serial_number;
}
absl::string_view Fru::GetPartNumber() const { return fru_info_.part_number; }

absl::flat_hash_map<std::string, Fru> CreateFrus(absl::Span<FruInstance> frus) {
  absl::flat_hash_map<std::string, Fru> frus_map;
  for (FruInstance &f : frus) {
    // Check that we don't have duplicate entries.
    assert(!frus_map.contains(f.fru_name));
    frus_map.emplace(std::move(f.fru_name), Fru(std::move(f.info)));
  }
  return frus_map;
}

}  // namespace ecclesia
