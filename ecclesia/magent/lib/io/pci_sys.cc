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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/substitute.h"
#include "absl/types/span.h"
#include "ecclesia/lib/apifs/apifs.h"
#include "ecclesia/lib/cleanup/cleanup.h"
#include "ecclesia/lib/codec/endian.h"
#include "ecclesia/lib/file/dir.h"
#include "ecclesia/lib/logging/logging.h"
#include "ecclesia/magent/lib/io/pci.h"
#include "ecclesia/magent/lib/io/pci_location.h"
#include "re2/re2.h"

namespace {

static LazyRE2 kBARLineRegexp = {
    R"((0x[0-9a-fA-F]{16}) (0x[0-9a-fA-F]{16}) (0x[0-9a-fA-F]{16}))"};

}  // namespace

namespace ecclesia {

namespace {
constexpr char kSysPciRoot[] = "/sys/bus/pci/devices";
constexpr size_t kMaxSysFileSize = 4096;

class DirCloser {
 public:
  explicit DirCloser(DIR *dir) : dir_(dir) {}
  DirCloser(const DirCloser &) = delete;
  DirCloser &operator=(const DirCloser &) = delete;
  ~DirCloser() { closedir(dir_); }

 private:
  DIR *dir_;
};

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

absl::Status SysPciRegion::ReadBytes(uint64_t offset, absl::Span<char> value) {
  return apifs_.SeekAndRead(offset, value);
}

absl::Status SysPciRegion::WriteBytes(uint64_t offset,
                                      absl::Span<const char> value) {
  return apifs_.SeekAndWrite(offset, value);
}

PciFunction::PciFunction(PciLocation loc) : loc_(loc) {}

SysfsPci::SysfsPci(PciLocation loc) : SysfsPci(kSysPciRoot, loc) {}
SysfsPci::SysfsPci(std::string sys_dir, PciLocation loc)
    : PciFunction{loc},
      // pass a copy to sys_dir_ since config_ will "move" sys_dir next.
      sys_dir_(sys_dir),
      config_(sys_dir, loc) {}

bool SysfsPci::Exists() {
  return sys_dir_.Exists(
      absl::StrFormat("devices/%s", absl::FormatStreamed(loc_)));
}

absl::Status SysfsPci::GetBaseAddress(BarNum bar_id,
                                      PciFunction::BAR *out_bar) const {

  std::string path = absl::StrFormat(
      "%s/devices/%s/resource", sys_dir_.GetPath(), absl::FormatStreamed(loc_));

  std::ifstream ifs(path, std::ifstream::in);

  if (!ifs.is_open()) {
    return absl::NotFoundError(absl::Substitute("Can't open path: $0", path));
  }

  // BAR is total 24 bytes(4bytes x 6 BAR) in config space
  // We are reading 'resource' file in sysfs.
  // The 'resource' file uses ascii string to represent hex values.
  // It has 13 lines, each line is 57 bytes long, '\0' terminated.
  // Example: /sys/bus/pci/devices/0000:3a:05.4/resource on indus.
  // 0x00000000b8700000 0x00000000b8700fff 0x0000000000040200
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  // 0x0000000000000000 0x0000000000000000 0x0000000000000000
  std::string line;
  std::vector<std::string> lines;
  while (getline(ifs, line)) {
    lines.push_back(line);
  }
  ifs.close();

  if (lines.empty() || lines.size() < kMaxBars) {
    return absl::InternalError(
        absl::Substitute("No bar information found in $0", path));
  }

  int64_t base;
  int64_t limit;
  int64_t flags;
  if (!RE2::FullMatch(lines[bar_id.value()], *kBARLineRegexp, RE2::Hex(&base),
                      RE2::Hex(&limit), RE2::Hex(&flags))) {
    return absl::InternalError(
        absl::Substitute("Error reading BAR information."));
  }

  out_bar->address = base;
  out_bar->type = (flags & kIoResourceIo) ? kBarTypeIo : kBarTypeMem;

  return absl::OkStatus();
}

SysfsPciDiscovery::SysfsPciDiscovery(std::string sys_dir) : sys_dir_(sys_dir) {}

absl::Status SysfsPciDiscovery::EnumerateAllDevices(
    std::vector<PciLocation> *devices) const {
  if (devices == nullptr) {
    return absl::InvalidArgumentError("devices can not be nullptr.");
  }

  std::string path = absl::StrCat(sys_dir_.c_str(), "/devices");

  absl::Status status =
      WithEachFileInDirectory(path, [devices](absl::string_view filename) {
        auto maybe_loc = PciLocation::FromString(filename);
        if (maybe_loc.has_value()) {
          devices->push_back(maybe_loc.value());
        }
      });
  if (!status.ok()) return status;

  std::sort(devices->begin(), devices->end());
  return absl::OkStatus();
}
}  // namespace ecclesia
