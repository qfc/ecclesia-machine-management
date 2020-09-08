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
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "ecclesia/lib/file/test_filesystem.h"
#include "ecclesia/lib/testing/status.h"
#include "ecclesia/magent/lib/io/pci.h"
#include "ecclesia/magent/lib/io/pci_location.h"

namespace ecclesia {
namespace {

using ::testing::Not;

using ::testing::UnorderedElementsAre;

// Helper matchers that can check if a BarInfo is a memory or I/O BAR with the
// given base address.
MATCHER_P(IsMemoryBar, expected_address, "") {
  return arg.type == PciResources::kBarTypeMem &&
         arg.address == expected_address;
}
MATCHER_P(IsIoBar, expected_address, "") {
  return arg.type == PciResources::kBarTypeIo &&
         arg.address == expected_address;
}

constexpr absl::string_view kTestResourceFile =
    R"(0x000000009d784000 0x000000009d785fff 0x0000000000040200
0x000000009d786000 0x000000009d7860ff 0x0000000000040200
0x0000000000003030 0x0000000000003037 0x0000000000040101
0x0000000000003020 0x0000000000003023 0x0000000000040101
0x0000000000003000 0x000000000000301f 0x0000000000040101
0x000000009d700000 0x000000009d77ffff 0x0000000000040200
0x0000000000000000 0x0000000000000000 0x0000000000000000
0x0000000000000000 0x0000000000000000 0x0000000000000000
0x0000000000000000 0x0000000000000000 0x0000000000000000
0x0000000000000000 0x0000000000000000 0x0000000000000000
0x0000000000000000 0x0000000000000000 0x0000000000000000
0x0000000000000000 0x0000000000000000 0x0000000000000000
0x0000000000000000 0x0000000000000000 0x0000000000000000)";

class PciSysTest : public testing::Test {
 public:
  PciSysTest() : fs_(GetTestTempdirPath()) {
    fs_.CreateDir("/sys/bus/pci/devices/0001:02:03.4");
    fs_.CreateFile("/sys/bus/pci/devices/0001:02:03.4/config", "0123456789");
    fs_.CreateFile("/sys/bus/pci/devices/0001:02:03.4/resource",
                   kTestResourceFile);
  }

 protected:
  TestFilesystem fs_;
};

class PciTopologyTest : public testing::Test {
 public:
  PciTopologyTest() : fs_(GetTestTempdirPath()) {
    fs_.CreateDir("/sys/devices");
    fs_.CreateDir("/sys/devices/pci0000:ae");
    fs_.CreateDir("/sys/devices/pci0000:ae/0000:ae:00.0");
    fs_.CreateDir("/sys/devices/pci0000:ae/0000:ae:00.0/0000:af:00.0");
    fs_.CreateDir("/sys/devices/pci0000:ae/0000:ae:00.0/0000:af:01.0");
    fs_.CreateDir("/sys/devices/pci0000:ae/0000:ae:01.0");
    fs_.CreateDir("/sys/devices/pci0000:d7");
    fs_.CreateDir("/sys/devices/pci0000:d7/0000.d7:00.0");
    fs_.CreateDir("/sys/devices/pci0000:ae/0000:d7:00.0/0000:d8:00.0");
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
  auto region = SysPciRegion(fs_.GetTruePath("/sys/bus/pci/devices/"), loc);

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
  auto region = SysPciRegion(fs_.GetTruePath("/sys/bus/pci/devices/"), loc);

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
  auto region = SysPciRegion(fs_.GetTruePath("/sys/bus/pci/devices/"), loc);

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
  auto region = SysPciRegion(fs_.GetTruePath("/sys/bus/pci/devices/"), loc);

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
  auto region = SysPciRegion(fs_.GetTruePath("/sys/bus/pci/devices/"), loc);
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

TEST_F(PciSysTest, TestResources) {
  auto loc = PciLocation::Make<1, 2, 3, 4>();
  auto resources =
      SysfsPciResources(fs_.GetTruePath("/sys/bus/pci/devices/"), loc);

  EXPECT_TRUE(resources.Exists());
  EXPECT_THAT(resources.GetBaseAddress<0>(),
              IsOkAndHolds(IsMemoryBar(0x9d784000)));
  EXPECT_THAT(resources.GetBaseAddress<1>(),
              IsOkAndHolds(IsMemoryBar(0x9d786000)));
  EXPECT_THAT(resources.GetBaseAddress<2>(), IsOkAndHolds(IsIoBar(0x3030)));
  EXPECT_THAT(resources.GetBaseAddress<3>(), IsOkAndHolds(IsIoBar(0x3020)));
  EXPECT_THAT(resources.GetBaseAddress<4>(), IsOkAndHolds(IsIoBar(0x3000)));
  EXPECT_THAT(resources.GetBaseAddress<5>(),
              IsOkAndHolds(IsMemoryBar(0x9d700000)));
}

TEST_F(PciSysTest, TestResourcesNotFound) {
  auto loc = PciLocation::Make<1, 2, 3, 5>();
  auto resources =
      SysfsPciResources(fs_.GetTruePath("/sys/bus/pci/devices/"), loc);

  EXPECT_FALSE(resources.Exists());
  EXPECT_THAT(resources.GetBaseAddress<0>(), Not(IsOk()));
  EXPECT_THAT(resources.GetBaseAddress<1>(), Not(IsOk()));
  EXPECT_THAT(resources.GetBaseAddress<2>(), Not(IsOk()));
  EXPECT_THAT(resources.GetBaseAddress<3>(), Not(IsOk()));
  EXPECT_THAT(resources.GetBaseAddress<4>(), Not(IsOk()));
  EXPECT_THAT(resources.GetBaseAddress<5>(), Not(IsOk()));
}

TEST_F(PciTopologyTest, EnumerateAllLocations) {
  SysfsPciDiscovery pci_discovery(fs_.GetTruePath("/sys/devices/"));
  auto pci_locations = pci_discovery.EnumerateAllLocations();
  std::vector<PciLocation> expect_all_locations = {
      PciLocation::Make<0, 0xae, 0, 0>(), PciLocation::Make<0, 0xae, 1, 0>(),
      PciLocation::Make<0, 0xaf, 0, 0>(), PciLocation::Make<0, 0xaf, 1, 0>(),
      PciLocation::Make<0, 0xd7, 0, 0>(), PciLocation::Make<0, 0xd8, 0, 0>()};
  // The enumerated PCI locations are sorted.
  EXPECT_THAT(pci_locations, expect_all_locations);
}

TEST_F(PciTopologyTest, GetCorrectRootNodes) {
  SysfsPciDiscovery pci_discovery(fs_.GetTruePath("/sys/devices/"));
  auto root_nodes = pci_discovery.GetRootNodes();
  std::vector<PciLocation> root_locations;
  for (auto* root_node : root_nodes) {
    ASSERT_NE(root_node, nullptr);
    root_locations.push_back(root_node->Location());
  }
  EXPECT_THAT(root_locations,
              UnorderedElementsAre(PciLocation::Make<0, 0xae, 0, 0>(),
                                   PciLocation::Make<0, 0xae, 1, 0>(),
                                   PciLocation::Make<0, 0xd7, 0, 0>()));
}

TEST_F(PciTopologyTest, GetCorrectNode) {
  SysfsPciDiscovery pci_discovery(fs_.GetTruePath("/sys/devices/"));
  std::vector<PciLocation> expect_all_locations = {
      PciLocation::Make<0, 0xae, 0, 0>(), PciLocation::Make<0, 0xae, 1, 0>(),
      PciLocation::Make<0, 0xaf, 0, 0>(), PciLocation::Make<0, 0xaf, 1, 0>(),
      PciLocation::Make<0, 0xd7, 0, 0>(), PciLocation::Make<0, 0xd8, 0, 0>()};
  // Verify GetNode returns correct node.
  for (const auto& location : expect_all_locations) {
    auto maybe_node = pci_discovery.GetNode(location);
    ASSERT_TRUE(maybe_node.ok());
    EXPECT_EQ(maybe_node.value()->Location(), location);
  }
  // GetNode returns status for non-exist PCI location
  auto maybe_node = pci_discovery.GetNode(PciLocation::Make<1, 2, 3, 4>());
  EXPECT_FALSE(maybe_node.ok());
}

TEST_F(PciTopologyTest, GetCorrectAncestorNode) {
  SysfsPciDiscovery pci_discovery(fs_.GetTruePath("/sys/devices/"));
  // 0000:ae:00.0 has two children 0000:af:00.0 and 0000:af:01.0
  auto maybe_node =
      pci_discovery.GetAncestorNode(PciLocation::Make<0, 0xaf, 0, 0>());
  ASSERT_TRUE(maybe_node.ok());
  EXPECT_EQ(maybe_node.value()->Location(),
            (PciLocation::Make<0, 0xae, 0, 0>()));
  maybe_node =
      pci_discovery.GetAncestorNode(PciLocation::Make<0, 0xaf, 1, 0>());
  ASSERT_TRUE(maybe_node.ok());
  EXPECT_EQ(maybe_node.value()->Location(),
            (PciLocation::Make<0, 0xae, 0, 0>()));
  EXPECT_EQ(maybe_node.value()->Parent(), nullptr);
  EXPECT_EQ(maybe_node.value()->Children().size(), 2);
}

}  // namespace
}  // namespace ecclesia
