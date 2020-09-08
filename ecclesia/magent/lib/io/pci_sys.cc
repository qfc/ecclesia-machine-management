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

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/lib/apifs/apifs.h"
#include "ecclesia/lib/codec/endian.h"
#include "ecclesia/lib/file/dir.h"
#include "ecclesia/lib/file/path.h"
#include "ecclesia/lib/logging/globals.h"
#include "ecclesia/lib/logging/logging.h"
#include "ecclesia/lib/types/fixed_range_int.h"
#include "ecclesia/magent/lib/io/pci.h"
#include "ecclesia/magent/lib/io/pci_location.h"
#include "re2/re2.h"

namespace ecclesia {
namespace {

constexpr char kSysPciRoot[] = "/sys/bus/pci/devices";
constexpr size_t kMaxSysFileSize = 4096;

static LazyRE2 kResourceLineRegex = {
    R"((0x[[:xdigit:]]{16}) (0x[[:xdigit:]]{16}) (0x[[:xdigit:]]{16}))"};

static LazyRE2 kPciBusEntryRegex = {
    R"(pci([[:xdigit:]]{4}):([[:xdigit:]]{2}))"};

}  // namespace

SysPciRegion::SysPciRegion(const PciLocation &loc)
    : SysPciRegion(kSysPciRoot, loc) {}

SysPciRegion::SysPciRegion(std::string sys_pci_devices_dir,
                           const PciLocation &loc)
    : PciRegion(kMaxSysFileSize),
      sys_pci_devices_dir_(std::move(sys_pci_devices_dir)),
      loc_(loc),
      apifs_(absl::StrFormat("%s/%s/config", sys_pci_devices_dir_,
                             absl::FormatStreamed(loc))) {}

absl::StatusOr<uint8_t> SysPciRegion::Read8(size_t offset) const {
  std::array<char, sizeof(uint8_t)> res;
  absl::Status status = apifs_.SeekAndRead(offset, absl::MakeSpan(res));

  if (!status.ok()) {
    return status;
  }

  return LittleEndian::Load8(res.data());
}

absl::Status SysPciRegion::Write8(size_t offset, uint8_t data) {
  char buffer[1];
  LittleEndian::Store8(data, buffer);

  return apifs_.SeekAndWrite(offset, absl::MakeConstSpan(buffer));
}

absl::StatusOr<uint16_t> SysPciRegion::Read16(size_t offset) const {
  std::array<char, sizeof(uint16_t)> res;
  absl::Status status = apifs_.SeekAndRead(offset, absl::MakeSpan(res));

  if (!status.ok()) {
    return status;
  }

  return LittleEndian::Load16(res.data());
}

absl::Status SysPciRegion::Write16(size_t offset, uint16_t data) {
  char buffer[2];
  LittleEndian::Store16(data, buffer);

  return apifs_.SeekAndWrite(offset, absl::MakeConstSpan(buffer));
}

absl::StatusOr<uint32_t> SysPciRegion::Read32(size_t offset) const {
  std::array<char, sizeof(uint32_t)> res;
  absl::Status status = apifs_.SeekAndRead(offset, absl::MakeSpan(res));

  if (!status.ok()) {
    return status;
  }

  return LittleEndian::Load32(res.data());
}

absl::Status SysPciRegion::Write32(size_t offset, uint32_t data) {
  char buffer[4];
  LittleEndian::Store32(data, buffer);

  return apifs_.SeekAndWrite(offset, absl::MakeConstSpan(buffer));
}

absl::Status SysPciRegion::ReadBytes(uint64_t offset,
                                     absl::Span<char> value) const {
  return apifs_.SeekAndRead(offset, value);
}

absl::Status SysPciRegion::WriteBytes(uint64_t offset,
                                      absl::Span<const char> value) {
  return apifs_.SeekAndWrite(offset, value);
}

SysfsPciResources::SysfsPciResources(PciLocation loc)
    : SysfsPciResources(kSysPciRoot, std::move(loc)) {}

SysfsPciResources::SysfsPciResources(std::string sys_pci_devices_dir,
                                     PciLocation loc)
    : PciResources(loc),
      apifs_(absl::StrFormat("%s/%s", sys_pci_devices_dir,
                             absl::FormatStreamed(loc))) {}

bool SysfsPciResources::Exists() { return apifs_.Exists(); }

absl::StatusOr<PciResources::BarInfo> SysfsPciResources::GetBaseAddressImpl(
    BarNum bar_id) const {
  ApifsFile resource_file(apifs_, "resource");
  auto maybe_contents = resource_file.Read();
  if (!maybe_contents.ok()) {
    return maybe_contents.status();
  }

  // BAR is total 24 bytes(4bytes x 6 BAR) in config space. We are reading
  // 'resource' file in sysfs. The 'resource' file uses ascii string to
  // represent hex values. Each line is 57 bytes long, '\0' terminated where the
  // first six lines correspond to the standard BAR resources.
  //
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
  std::vector<absl::string_view> lines =
      absl::StrSplit(absl::StripSuffix(*maybe_contents, "\n"), '\n');

  if (lines.size() < BarNum::kMaxValue) {
    return absl::InternalError(absl::Substitute(
        "No bar information found in $0", resource_file.GetPath()));
  }

  uint64_t base, limit, flags;
  if (!RE2::FullMatch(lines[bar_id.value()], *kResourceLineRegex,
                      RE2::Hex(&base), RE2::Hex(&limit), RE2::Hex(&flags))) {
    return absl::InternalError(
        absl::Substitute("Error reading BAR information."));
  }

  return BarInfo{(flags & kIoResourceFlag) ? kBarTypeIo : kBarTypeMem, base};
}

SysfsPciDiscovery::SysfsPciDiscovery(const std::string &sys_devices_dir)
    : sys_devices_dir_(sys_devices_dir) {
  auto status = Rescan();
  if (!status.ok()) {
    ecclesia::ErrorLog() << "PCI topology construction failed. Status: "
                         << status.ToString();
  }
}

std::vector<PciLocation> SysfsPciDiscovery::EnumerateAllLocations() const {
  std::vector<PciLocation> pci_locations;
  for (auto &entry : pci_table_) {
    pci_locations.push_back(entry.first);
  }
  std::sort(pci_locations.begin(), pci_locations.end());
  return pci_locations;
}

absl::Status SysfsPciDiscovery::Rescan() {
  absl::Status status = WithEachFileInDirectory(
      sys_devices_dir_, [&](absl::string_view entry_name) {
        if (RE2::FullMatch(entry_name, *kPciBusEntryRegex)) {
          ScanDirectory(JoinFilePaths(sys_devices_dir_, entry_name), 0,
                        nullptr);
        }
      });
  return status;
}

std::vector<PciTopologyNode *> SysfsPciDiscovery::ScanDirectory(
    absl::string_view directory_path, size_t depth, PciTopologyNode *parent) {
  std::vector<PciTopologyNode *> nodes_in_this_dir;
  absl::Status status = WithEachFileInDirectory(
      directory_path, [&](absl::string_view entry_name) {
        auto maybe_loc = PciLocation::FromString(entry_name);
        if (maybe_loc.has_value()) {
          auto node = std::make_unique<PciTopologyNode>(maybe_loc.value(),
                                                        depth, parent);
          node->SetChildren(
              ScanDirectory(JoinFilePaths(directory_path, entry_name),
                            depth + 1, node.get()));

          nodes_in_this_dir.push_back(node.get());
          pci_table_.emplace(maybe_loc.value(), std::move(node));
        }
      });
  return nodes_in_this_dir;
}

std::vector<PciTopologyNode *> SysfsPciDiscovery::GetRootNodes() const {
  std::vector<PciTopologyNode *> root_nodes;
  for (auto &entry : pci_table_) {
    if (entry.second->Depth() == 0) {
      root_nodes.push_back(entry.second.get());
    }
  }
  return root_nodes;
}

absl::StatusOr<PciTopologyNode *> SysfsPciDiscovery::GetNode(
    const PciLocation &location) const {
  auto iter = pci_table_.find(location);
  if (iter != pci_table_.end()) {
    return iter->second.get();
  }
  return absl::NotFoundError(
      absl::StrCat("Failed to find PCI node: ", location.ToString()));
}
}  // namespace ecclesia
