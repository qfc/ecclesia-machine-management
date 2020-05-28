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

// This library provides support for interacting with Model Specific Regisger.

#ifndef ECCLESIA_LIB_IO_MSR_H_
#define ECCLESIA_LIB_IO_MSR_H_

#include <cstdint>
#include <filesystem>
#include <string>

#include "absl/status/status.h"

namespace ecclesia {

class Msr {
 public:
  // Construct an object for accessing msr at location specified by path.
  explicit Msr(std::string path);

  // Msr access objects cannot be copied or moved.
  Msr(Msr &&other) = delete;
  Msr(const Msr &other) = delete;
  Msr &operator=(Msr &&other) = delete;
  Msr &operator=(const Msr &other) = delete;

  // Read the value from a specific MSR.
  absl::Status Read(uint64_t reg, uint64_t *value) const;

  // Write a value to a specific MSR.
  absl::Status Write(uint64_t reg, uint64_t value) const;

 private:
  // Indicates if the given path exists.
  bool Exists() const;

  std::filesystem::path path_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_IO_MSR_H_
