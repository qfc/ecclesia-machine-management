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

#include <cstdint>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/numeric/int128.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ecclesia/magent/lib/nvme/nvme_types.h"

namespace ecclesia {
namespace {

using ::testing::ElementsAreArray;

// IdentifyNamespace data collected from NVMe SSD with nvme-cli, like so:
// mvee4:~# ./nvme id-ns /mnt/devtmpfs/nvme0 -n 8 -b | xxd -i
constexpr unsigned char kIdentifyNamespaceData[4096] = {
    0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x5c, 0xd2, 0xe4, 0x57, 0xdf, 0x82, 0x50, 0x51,
    0x5c, 0xd2, 0xe4, 0x57, 0xdf, 0x82, 0x09, 0x00, 0x00, 0x00, 0x09, 0x02,
    0x00, 0x00, 0x0c, 0x00,
};

bool operator==(const LBAFormat& lhs, const LBAFormat& rhs) {
  return std::tie(lhs.metadata_size, lhs.data_size, lhs.relative_performance) ==
         std::tie(rhs.metadata_size, rhs.data_size, rhs.relative_performance);
}

TEST(IdentifyNamespaceTest, ParseIdentifyData) {
  auto ret = IdentifyNamespace::Parse(std::string(
      kIdentifyNamespaceData,
      kIdentifyNamespaceData +
          sizeof(kIdentifyNamespaceData) / sizeof(kIdentifyNamespaceData[0])));
  ASSERT_TRUE(ret.ok());
  const auto id = std::move(ret.value());

  constexpr uint64_t expected_lba_size_bytes = 4096;
  constexpr uint64_t expected_size_blocks = 0x40000;
  constexpr uint64_t expected_size_bytes =
      expected_size_blocks * expected_lba_size_bytes;

  EXPECT_EQ(expected_lba_size_bytes, id.formatted_lba_size_bytes);
  EXPECT_EQ(expected_size_bytes, id.capacity_bytes);
  EXPECT_EQ(absl::MakeUint128(0x0800000001000000, 0x5cd2e457df825051),
            id.namespace_guid);
}

TEST(IdentifyListNamespacesParseTest, ParseInvalidBuffer) {
  auto ret = ParseIdentifyListNamespace(std::string(1024, '\0'));
  EXPECT_EQ(absl::StatusCode::kInvalidArgument, ret.status().code());
}

TEST(IdentifyListNamespacesParseTest, ParseEmptyList) {
  auto ret = ParseIdentifyListNamespace(std::string(4096, '\0'));
  ASSERT_TRUE(ret.ok());
  EXPECT_THAT(ret.value(), std::set<uint32_t>());
}

TEST(IdentifyListNamespacesParseTest, ParseSmallList) {
  constexpr char short_buf[4096] = {
      0x02, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
  };
  auto ret = ParseIdentifyListNamespace(std::string(short_buf, 4096));
  ASSERT_TRUE(ret.ok());
  auto set = std::set<uint32_t>{2, 8, 256, 67305985};
  EXPECT_THAT(ret.value(), set);
}

TEST(IdentifyNamespaceTest, ListSupportedFormats) {
  auto ret = IdentifyNamespace::GetSupportedLbaFormats(std::string(
      kIdentifyNamespaceData,
      kIdentifyNamespaceData +
          sizeof(kIdentifyNamespaceData) / sizeof(kIdentifyNamespaceData[0])));
  ASSERT_TRUE(ret.ok());
  const auto supported_formats = ret.value();

  const std::vector<LBAFormat> expected_formats{
      {0, 9, 2},
      {0, 12, 0},
  };

  // Note that order matters, so we don't use Unordered here.
  EXPECT_THAT(supported_formats, ElementsAreArray(expected_formats));
}

// Check we return an error if the buffer contains no valid formats.
TEST(IdentifyNamespaceTest, ListSupportedFormatsEmpty) {
  auto ret = IdentifyNamespace::GetSupportedLbaFormats(
      std::string(sizeof(kIdentifyNamespaceData), '\0'));
  EXPECT_EQ(absl::StatusCode::kNotFound, ret.status().code());
}

}  // namespace
}  // namespace ecclesia
