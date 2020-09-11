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

#include "ecclesia/magent/sysmodel/x86/chassis.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "ecclesia/magent/lib/io/usb_mock.h"

namespace ecclesia {
namespace {

using ::testing::ElementsAre;
using ::testing::Return;

TEST(ChassisIdTest, GetChassisStringAndType) {
  EXPECT_EQ(ChassisIdToString(ChassisId::kIndus), "Indus");
  EXPECT_EQ(GetChassisType(ChassisId::kIndus), ChassisType::kRackMount);
  EXPECT_EQ(GetChassisTypeAsString(ChassisId::kIndus), "RackMount");
  EXPECT_EQ(ChassisIdToString(ChassisId::kSleipnir), "Sleipnir");
  EXPECT_EQ(GetChassisType(ChassisId::kSleipnir),
            ChassisType::kStorageEnclosure);
  EXPECT_EQ(GetChassisTypeAsString(ChassisId::kSleipnir), "StorageEnclosure");
}

TEST(DetectChassisByUsbTest, DetectUsbChassis) {
  MockUsbDiscovery usb_discovery;
  auto sleipnir_bmc = std::make_unique<MockUsbDevice>();
  UsbSignature sleipnir_bmc_usb_sig = {0x18d1, 0x0215};
  EXPECT_CALL(*sleipnir_bmc, GetSignature())
      .WillOnce(Return(sleipnir_bmc_usb_sig));
  std::vector<std::unique_ptr<UsbDeviceIntf>> usb_devices;
  usb_devices.push_back(std::move(sleipnir_bmc));
  EXPECT_CALL(usb_discovery, EnumerateAllUsbDevices()).WillOnce([&]() {
    return std::move(usb_devices);
  });

  auto maybe_chassisid = DetectChassisByUsb(&usb_discovery);
  ASSERT_TRUE(maybe_chassisid.has_value());
  EXPECT_EQ(maybe_chassisid.value(), ChassisId::kSleipnir);
}

TEST(DetectChassisByUsbTest, NoUsbChassisFound) {
  MockUsbDiscovery usb_discovery;
  std::vector<std::unique_ptr<UsbDeviceIntf>> usb_devices;
  EXPECT_CALL(usb_discovery, EnumerateAllUsbDevices())
      .WillOnce([&]() { return std::move(usb_devices); })
      .WillOnce([]() { return absl::InternalError("Some error"); });

  // The first time no chassis found is due to empty USB devices.
  auto maybe_chassisid = DetectChassisByUsb(&usb_discovery);
  EXPECT_FALSE(maybe_chassisid.has_value());
  // The second time no chassis found is due to EnumerateAllUsbDevices error.
  maybe_chassisid = DetectChassisByUsb(&usb_discovery);
  EXPECT_FALSE(maybe_chassisid.has_value());
}

TEST(CreateChassisTest, CreateServerAndUsbChassis) {
  MockUsbDiscovery usb_discovery;
  auto sleipnir_bmc = std::make_unique<MockUsbDevice>();
  UsbSignature sleipnir_bmc_usb_sig = {0x18d1, 0x0215};
  EXPECT_CALL(*sleipnir_bmc, GetSignature())
      .WillOnce(Return(sleipnir_bmc_usb_sig));
  std::vector<std::unique_ptr<UsbDeviceIntf>> usb_devices;
  usb_devices.push_back(std::move(sleipnir_bmc));
  EXPECT_CALL(usb_discovery, EnumerateAllUsbDevices()).WillOnce([&]() {
    return std::move(usb_devices);
  });

  std::vector<ChassisId> chassis_ids = CreateChassis(&usb_discovery);
  EXPECT_THAT(chassis_ids,
              ElementsAre(ChassisId::kIndus, ChassisId::kSleipnir));
}

}  // namespace
}  // namespace ecclesia
