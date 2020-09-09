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

#include <cstdint>
#include <string>

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ecclesia/lib/file/test_filesystem.h"
#include "ecclesia/magent/lib/io/pci_location.h"

namespace ecclesia {
namespace {

class PciSysTest : public testing::Test {
 public:
  PciSysTest() : fs_(GetTestTempdirPath()) {
    fs_.CreateDir("/bus/pci/devices/0001:02:03.4");
    fs_.CreateFile("/bus/pci/devices/0001:02:03.4/config", "0123456789");
  }

 protected:
  TestFilesystem fs_;
};

TEST_F(PciSysTest, TestRegionDefaultConstructed) {
  auto loc = PciLocation::Make<1, 2, 3, 4>();
  auto region = SysPciRegion(loc);

  auto maybe_uint8 = region.Read8(4);
  EXPECT_TRUE(absl::IsNotFound(maybe_uint8.status()));
}

TEST_F(PciSysTest, TestRead) {
  auto loc = PciLocation::Make<1, 2, 3, 4>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  auto region = SysPciRegion(std::move(path), loc);

  auto maybe_uint8 = region.Read8(4);
  EXPECT_TRUE(maybe_uint8.ok());
  EXPECT_EQ(maybe_uint8.value(), 0x34);

  auto maybe_uint16 = region.Read16(4);
  EXPECT_TRUE(maybe_uint16.ok());
  EXPECT_EQ(maybe_uint16.value(), 0x3534);

  auto maybe_uint32 = region.Read32(4);
  EXPECT_TRUE(maybe_uint32.ok());
  EXPECT_EQ(maybe_uint32.value(), 0x37363534);
}

TEST_F(PciSysTest, TestReadFailOutofRange) {
  auto loc = PciLocation::Make<1, 2, 3, 4>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  SysPciRegion region(std::move(path), loc);

  auto maybe_uint8 = region.Read8(10);
  EXPECT_FALSE(maybe_uint8.ok());
  EXPECT_TRUE(absl::IsInternal(maybe_uint8.status()));

  auto maybe_uint16 = region.Read16(10);
  EXPECT_FALSE(maybe_uint16.ok());
  EXPECT_TRUE(absl::IsInternal(maybe_uint16.status()));

  auto maybe_uint32 = region.Read32(10);
  EXPECT_FALSE(maybe_uint32.ok());
  EXPECT_TRUE(absl::IsInternal(maybe_uint32.status()));
}

TEST_F(PciSysTest, TestReadFailNotFound) {
  auto loc = PciLocation::Make<1, 2, 3, 5>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  SysPciRegion region(std::move(path), loc);

  auto maybe_uint8 = region.Read8(10);
  EXPECT_FALSE(maybe_uint8.ok());
  EXPECT_TRUE(absl::IsNotFound(maybe_uint8.status()));

  auto maybe_uint16 = region.Read16(10);
  EXPECT_FALSE(maybe_uint16.ok());
  EXPECT_TRUE(absl::IsNotFound(maybe_uint16.status()));

  auto maybe_uint32 = region.Read32(10);
  EXPECT_FALSE(maybe_uint32.ok());
  EXPECT_TRUE(absl::IsNotFound(maybe_uint32.status()));
}

TEST_F(PciSysTest, TestWrite) {
  auto loc = PciLocation::Make<1, 2, 3, 4>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  SysPciRegion region(std::move(path), loc);

  uint8_t u8 = 0x0B;
  EXPECT_TRUE(region.Write8(4, u8).ok());
  auto maybe_uint8 = region.Read8(4);
  EXPECT_TRUE(maybe_uint8.ok());
  EXPECT_EQ(maybe_uint8.value(), u8);

  uint16_t u16 = 0xBE;
  EXPECT_TRUE(region.Write16(4, u16).ok());
  auto maybe_uint16 = region.Read16(4);
  EXPECT_TRUE(maybe_uint16.ok());
  EXPECT_EQ(maybe_uint16.value(), u16);

  uint32_t u32 = 0xBEEF;
  EXPECT_TRUE(region.Write32(4, u32).ok());
  auto maybe_uint32 = region.Read32(4);
  EXPECT_TRUE(maybe_uint32.ok());
  EXPECT_EQ(maybe_uint32.value(), u32);
}

TEST_F(PciSysTest, TestWriteFailNotFound) {
  auto loc = PciLocation::Make<1, 2, 3, 5>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  SysPciRegion region(std::move(path), loc);
  absl::Status status;

  uint8_t u8 = 0x0B;
  status = region.Write8(4, u8);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsNotFound(status));

  uint16_t u16 = 0x0BE;
  status = region.Write16(4, u16);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsNotFound(status));

  uint32_t u32 = 0x0BEEF;
  status = region.Write32(4, u32);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsNotFound(status));
}

}  // namespace
}  // namespace ecclesia
