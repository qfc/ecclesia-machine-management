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

#include "lib/io/msr.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "lib/apifs/apifs.h"
#include "lib/codec/endian.h"

namespace ecclesia {

namespace fs = ::std::filesystem;

Msr::Msr(std::string path) : path_(std::move(path)) {}

bool Msr::Exists() const {
  std::error_code ec;  // Ignored on error, we return false.
  return fs::exists(path_, ec);
}

absl::Status Msr::Read(uint64_t reg, uint64_t *value) const {
  ApifsFile apifs(path_);
  std::array<char, sizeof(uint64_t)> res;
  absl::Status status = apifs.SeekAndRead(reg, absl::MakeSpan(res));
  if (!status.ok()) {
    return status;
  }

  *value = LittleEndian::Load64(res.data());
  return absl::OkStatus();
}

absl::Status Msr::Write(uint64_t reg, uint64_t value) const {
  char buffer[8];
  LittleEndian::Store64(value, buffer);

  ApifsFile apifs(path_);
  return apifs.SeekAndWrite(reg, absl::MakeConstSpan(buffer));
}

}  // namespace ecclesia
