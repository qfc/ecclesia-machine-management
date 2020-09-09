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

#include "ecclesia/magent/lib/nvme/identify_namespace.h"

#include <string.h>

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "absl/numeric/int128.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "ecclesia/lib/codec/endian.h"
#include "ecclesia/magent/lib/nvme/nvme_types.h"

namespace ecclesia {

absl::StatusOr<IdentifyNamespace> IdentifyNamespace::Parse(
    absl::string_view buf) {
  if (buf.size() != kIdentifyNamespaceSize) {
    return absl::InvalidArgumentError(
        "Buffer was the wrong size for an IdentifyNamespace command.");
  }

  const IdentifyNamespaceFormat *const idns =
      reinterpret_cast<const IdentifyNamespaceFormat *>(&buf[0]);

  // Bits 3:0 indicates the supported LBA formats. Bit 4 indicate if
  // metadata is transferred at the end of the data LBA.
  const auto lba_format_index = (idns->formatted_lba_size) & 0xf;
  if (lba_format_index > kIdentifyNamespaceLbaFormatCapacity) {
    return absl::OutOfRangeError(absl::StrCat(
        "Namespace had invalid formatted_lba_size index: ", lba_format_index));
  }
  const LBAFormat &lba_format = idns->lba_format[lba_format_index];
  const uint64_t lba_bytes =
      (lba_format.data_size > 0) ? (1ULL << lba_format.data_size) : 0;

  IdentifyNamespace id;
  id.capacity_bytes =
      LittleEndian::Load64(
          reinterpret_cast<const unsigned char *>(&idns->capacity)) *
      lba_bytes;
  id.formatted_lba_size_bytes = lba_bytes;
  id.namespace_guid = BigEndian::Load128(
      reinterpret_cast<const unsigned char *>(&idns->namespace_guid_low));

  return id;
}

absl::StatusOr<std::vector<LBAFormat>>
IdentifyNamespace::GetSupportedLbaFormats(absl::string_view buf) {
  if (buf.size() != kIdentifyNamespaceSize) {
    return absl::InvalidArgumentError(
        "Buffer was the wrong size for an IdentifyNamespace command.");
  }

  const IdentifyNamespaceFormat *const idns =
      reinterpret_cast<const IdentifyNamespaceFormat *>(&buf[0]);

  std::vector<LBAFormat> supported_formats;

  for (const LBAFormat &lba_format : idns->lba_format) {
    if (lba_format.data_size == 0) break;

    LBAFormat supported_format_host = lba_format;
    supported_format_host.metadata_size = BigEndian::Load16(
        reinterpret_cast<const unsigned char *>(&lba_format.metadata_size));

    supported_formats.push_back(supported_format_host);
  }

  if (supported_formats.empty()) {
    return absl::NotFoundError("Drive reported 0 supported formats.");
  }

  return supported_formats;
}

bool operator!=(const IdentifyNamespace &lhs, const IdentifyNamespace &rhs) {
  return memcmp(&lhs, &rhs, sizeof(lhs));
}

bool operator==(const IdentifyNamespace &lhs, const IdentifyNamespace &rhs) {
  return !(lhs != rhs);
}

absl::StatusOr<std::set<uint32_t>> ParseIdentifyListNamespace(
    absl::string_view buf) {
  if (buf.size() != kIdentifyListNamespaceSize) {
    return absl::InvalidArgumentError(
        "Input buffer was not the correct size for an IdentifyListNamespaces "
        "command.");
  }

  const IdentifyListNamespaceFormat *const idnsf =
      reinterpret_cast<const IdentifyListNamespaceFormat *>(buf.data());

  std::set<uint32_t> valid_nsids;

  for (uint32_t i = 0; i < kIdentifyListNamespacesCapacity; ++i) {
    const uint32_t nsid = LittleEndian::Load32(
        reinterpret_cast<const unsigned char *>(&idnsf->nsid[i]));
    if (nsid == 0) break;
    valid_nsids.insert(nsid);
  }

  return valid_nsids;
}

}  // namespace ecclesia
