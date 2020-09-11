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

#ifndef ECCLESIA_MAGENT_LIB_IO_USB_MOCK_H_
#define ECCLESIA_MAGENT_LIB_IO_USB_MOCK_H_

#include <memory>
#include <vector>

#include "gmock/gmock.h"
#include "absl/status/statusor.h"
#include "ecclesia/magent/lib/io/usb.h"

namespace ecclesia {

class MockUsbDiscovery : public UsbDiscoveryInterface {
 public:
  MOCK_METHOD(absl::StatusOr<std::vector<std::unique_ptr<UsbDeviceIntf>>>,
              EnumerateAllUsbDevices, (), (const, override));
};

class MockUsbDevice : public UsbDeviceIntf {
 public:
  MOCK_METHOD(const UsbLocation &, Location, (), (const, override));

  MOCK_METHOD(absl::StatusOr<UsbSignature>, GetSignature, (),
              (const, override));
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_USB_MOCK_H_
