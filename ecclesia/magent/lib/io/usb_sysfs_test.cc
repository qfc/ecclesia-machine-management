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

#include "ecclesia/magent/lib/io/usb_sysfs.h"

#include <cstddef>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "ecclesia/lib/file/test_filesystem.h"
#include "ecclesia/magent/lib/io/usb.h"

namespace ecclesia {
namespace {

class SysfsUsbDiscoveryTest : public testing::Test {
 protected:
  SysfsUsbDiscoveryTest()
      : fs_(GetTestTempdirPath()), usb_(GetTestTempdirPath() + "/sys") {
    fs_.CreateDir("/sys/bus/usb/devices");

    // Create some USB devices.
    CreateDevice("usb1", "0000:00:1a.1");
    CreateDevice("usb2", "0000:00:1a.2");
    CreateDevice("usb3", "0000:00:1d.1");
    CreateDevice("2-1", "0000:00:1a.2/usb2");
    CreateDevice("2-2", "0000:00:1a.2/usb2");
    CreateDevice("2-3", "0000:00:1a.2/usb2");
    CreateDevice("2-2.1", "0000:00:1a.2/usb2/2-2");
    CreateDevice("2-2.4", "0000:00:1a.2/usb2/2-2");
    CreateDevice("2-2.5", "0000:00:1a.2/usb2/2-2");
    CreateDevice("2-2.15", "0000:00:1a.2/usb2/2-2");
    CreateDevice("2-2.5.10", "0000:00:1a.2/usb2/2-2");
  }

  void CreateDevice(const std::string &devname, const std::string &pci) {
    std::string real_devname = "/sys/devices/pciXXXX:XX/" + pci + "/" + devname;
    fs_.CreateDir(real_devname);
    fs_.CreateSymlink(real_devname, "/sys/bus/usb/devices/" + devname);
  }

  TestFilesystem fs_;
  SysfsUsbDiscovery usb_;
};

// Helper that return true if a matching DeviceLocation in a vector, or
// returns false if the location isn't found.
bool FindUsbLocation(const std::vector<UsbLocation> &devices,
                     const UsbLocation &loc) {
  for (size_t i = 0; i < devices.size(); ++i) {
    if (loc == devices[i]) {
      return true;
    }
  }
  return false;
}

TEST_F(SysfsUsbDiscoveryTest, TestDeviceEnumeration) {
  std::vector<UsbLocation> usb_devices;
  ASSERT_TRUE(usb_.EnumerateAllUsbDevices(&usb_devices).ok());
  EXPECT_EQ(usb_devices.size(), 11);
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<1>(), UsbPortSequence())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(), UsbPortSequence())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<3>(), UsbPortSequence())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(),
                               UsbPortSequence::TryMake({1}).value())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(),
                               UsbPortSequence::TryMake({2}).value())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(),
                               UsbPortSequence::TryMake({3}).value())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(),
                               UsbPortSequence::TryMake({2, 1}).value())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(),
                               UsbPortSequence::TryMake({2, 4}).value())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(),
                               UsbPortSequence::TryMake({2, 5}).value())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(),
                               UsbPortSequence::TryMake({2, 15}).value())));
  EXPECT_TRUE(FindUsbLocation(
      usb_devices, UsbLocation(UsbBusLocation::Make<2>(),
                               UsbPortSequence::TryMake({2, 5, 10}).value())));
}

}  // namespace
}  // namespace ecclesia
