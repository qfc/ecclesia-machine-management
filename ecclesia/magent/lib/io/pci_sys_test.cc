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

  uint8_t u8;
  auto status = region.Read8(4, &u8);
  EXPECT_TRUE(absl::IsNotFound(status));
}

TEST_F(PciSysTest, TestRead) {
  auto loc = PciLocation::Make<1, 2, 3, 4>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  auto region = SysPciRegion(path, loc);

  uint8_t u8;
  EXPECT_TRUE(region.Read8(4, &u8).ok());
  EXPECT_EQ(u8, 0x34);

  uint16_t u16;
  EXPECT_TRUE(region.Read16(4, &u16).ok());
  EXPECT_EQ(u16, 0x3534);

  uint32_t u32;
  EXPECT_TRUE(region.Read32(4, &u32).ok());
  EXPECT_EQ(u32, 0x37363534);
}

TEST_F(PciSysTest, TestReadFailOutofRange) {
  auto loc = PciLocation::Make<1, 2, 3, 4>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  auto region = SysPciRegion(path, loc);
  absl::Status status;

  uint8_t u8;
  status = region.Read8(10, &u8);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsInternal(status));

  uint16_t u16;
  status = region.Read16(10, &u16);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsInternal(status));

  uint32_t u32;
  status = region.Read32(10, &u32);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsInternal(status));
}

TEST_F(PciSysTest, TestReadFailNotFound) {
  auto loc = PciLocation::Make<1, 2, 3, 5>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  auto region = SysPciRegion(path, loc);
  absl::Status status;

  uint8_t u8;
  status = region.Read8(10, &u8);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsNotFound(status));

  uint16_t u16;
  status = region.Read16(10, &u16);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsNotFound(status));

  uint32_t u32;
  status = region.Read32(10, &u32);
  EXPECT_FALSE(status.ok());
  EXPECT_TRUE(absl::IsNotFound(status));
}

TEST_F(PciSysTest, TestWrite) {
  auto loc = PciLocation::Make<1, 2, 3, 4>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  auto region = SysPciRegion(path, loc);

  uint8_t u8 = 0x0B, expected8 = 0;
  EXPECT_TRUE(region.Write8(4, u8).ok());
  EXPECT_TRUE(region.Read8(4, &expected8).ok());
  EXPECT_EQ(u8, expected8);

  uint16_t u16 = 0xBE, expected16 = 0;
  EXPECT_TRUE(region.Write16(4, u16).ok());
  EXPECT_TRUE(region.Read16(4, &expected16).ok());
  EXPECT_EQ(u16, expected16);

  uint32_t u32 = 0xBEEF, expected32 = 0;
  EXPECT_TRUE(region.Write32(4, u32).ok());
  EXPECT_TRUE(region.Read32(4, &expected32).ok());
  EXPECT_EQ(u32, expected32);
}

TEST_F(PciSysTest, TestWriteFailNotFound) {
  auto loc = PciLocation::Make<1, 2, 3, 5>();
  std::string path = GetTestTempdirPath() + "/bus/pci/devices/";
  auto region = SysPciRegion(path, loc);
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
