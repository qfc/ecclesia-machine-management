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

#include "ecclesia/magent/lib/io/pci_sys.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "ecclesia/lib/apifs/apifs.h"
#include "ecclesia/lib/codec/endian.h"
#include "ecclesia/magent/lib/io/pci.h"
#include "ecclesia/magent/lib/io/pci_location.h"

namespace ecclesia {

namespace {
constexpr char kSysPciRoot[] = "/sys/bus/pci/devices";
constexpr size_t kMaxSysFileSize = 4096;
}  // namespace

SysPciRegion::SysPciRegion(PciLocation loc) : SysPciRegion(kSysPciRoot, loc) {}

SysPciRegion::SysPciRegion(std::string sys_root, PciLocation loc)
    : PciRegion(kMaxSysFileSize),
      sys_root_(std::move(sys_root)),
      loc_(loc),
      apifs_(absl::StrFormat("%s/%s/config", sys_root_,
                             absl::FormatStreamed(loc))) {}

absl::Status SysPciRegion::Read8(size_t offset, uint8_t *data) {
  std::array<char, sizeof(uint8_t)> res;
  absl::Status status = apifs_.SeekAndRead(offset, absl::MakeSpan(res));

  if (!status.ok()) {
    return status;
  }

  *data = LittleEndian::Load8(res.data());
  return absl::OkStatus();
}

absl::Status SysPciRegion::Write8(size_t offset, uint8_t data) {
  char buffer[1];
  LittleEndian::Store8(data, buffer);

  return apifs_.SeekAndWrite(offset, absl::MakeConstSpan(buffer));
}

absl::Status SysPciRegion::Read16(size_t offset, uint16_t *data) {
  std::array<char, sizeof(uint16_t)> res;
  absl::Status status = apifs_.SeekAndRead(offset, absl::MakeSpan(res));

  if (!status.ok()) {
    return status;
  }

  *data = LittleEndian::Load16(res.data());
  return absl::OkStatus();
}

absl::Status SysPciRegion::Write16(size_t offset, uint16_t data) {
  char buffer[2];
  LittleEndian::Store16(data, buffer);

  return apifs_.SeekAndWrite(offset, absl::MakeConstSpan(buffer));
}

absl::Status SysPciRegion::Read32(size_t offset, uint32_t *data) {
  std::array<char, sizeof(uint32_t)> res;
  absl::Status status = apifs_.SeekAndRead(offset, absl::MakeSpan(res));

  if (!status.ok()) {
    return status;
  }

  *data = LittleEndian::Load32(res.data());
  return absl::OkStatus();
}

absl::Status SysPciRegion::Write32(size_t offset, uint32_t data) {
  char buffer[4];
  LittleEndian::Store32(data, buffer);

  return apifs_.SeekAndWrite(offset, absl::MakeConstSpan(buffer));
}

}  // namespace ecclesia
