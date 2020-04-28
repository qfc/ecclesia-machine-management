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

#ifndef ECCLESIA_MAGENT_SYSMODEL_FRU_H_
#define ECCLESIA_MAGENT_SYSMODEL_FRU_H_

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
namespace ecclesia {

struct FruInfo {
  std::string manufacturer;
  std::string serial_number;
  std::string part_number;
};

// TODO(dwangsf): Remove FruInstance once the FRU information is no longer
// hardcoded but rather read from SMBUS.
struct FruInstance {
  std::string fru_name;
  FruInfo info;
};

class Fru {
 public:
  Fru(FruInfo fru_info);

  // Allow the object to be copyable
  // Make sure that copy construction is relatively light weight.
  // In cases where it is not feasible to copy construct data members,it may
  // make sense to wrap the data member in a shared_ptr.
  Fru(const Fru &dimm) = default;
  Fru &operator=(const Fru &dimm) = default;

  absl::string_view GetManufacturer() const;
  absl::string_view GetSerialNumber() const;
  absl::string_view GetPartNumber() const;

 private:
  FruInfo fru_info_;
};

// TODO(dwangsf): Update this constructor with readers. For now, just accept
// hardcoded info to construct Frus.
absl::flat_hash_map<std::string, Fru> CreateFrus(absl::Span<FruInstance> frus);

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_SYSMODEL_FRU_H_
