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

#ifndef ECCLESIA_MAGENT_LIB_IO_USB_SYSFS_H_
#define ECCLESIA_MAGENT_LIB_IO_USB_SYSFS_H_

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "ecclesia/lib/apifs/apifs.h"
#include "ecclesia/magent/lib/io/usb.h"

namespace ecclesia {

extern const char kUsbDevicesDir[];

// Convert a usb device location to the sysfs directory name used for that
// device.
std::string UsbLocationToDirectory(const UsbLocation &loc);

class SysfsUsbDiscovery : public UsbDiscoveryInterface {
 public:
  SysfsUsbDiscovery() : SysfsUsbDiscovery(ApifsDirectory(kUsbDevicesDir)) {}
  // This constructor is for testing purpose.
  explicit SysfsUsbDiscovery(const ApifsDirectory &api_fs) : api_fs_(api_fs) {}

  SysfsUsbDiscovery(const SysfsUsbDiscovery &other) = delete;
  SysfsUsbDiscovery &operator=(const SysfsUsbDiscovery &other) = delete;

  absl::Status EnumerateAllUsbDevices(
      std::vector<UsbLocation> *devices) const override;

 private:
  ApifsDirectory api_fs_;
};

class SysfsUsbAccess : public UsbAccessInterface {
 public:
  SysfsUsbAccess(const UsbLocation &usb_location)
      : SysfsUsbAccess(
            usb_location,
            ApifsDirectory(absl::StrCat(
                kUsbDevicesDir, "/", UsbLocationToDirectory(usb_location)))) {}
  // This constructor is for testing purpose.
  SysfsUsbAccess(const UsbLocation &usb_location, const ApifsDirectory &api_fs)
      : usb_location_(usb_location), api_fs_(api_fs) {}

  SysfsUsbAccess(const SysfsUsbAccess &other) = delete;
  SysfsUsbAccess &operator=(const SysfsUsbAccess &other) = delete;

  absl::StatusOr<UsbSignature> GetSignature() const override;

 private:
  UsbLocation usb_location_;
  ApifsDirectory api_fs_;
};
}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_USB_SYSFS_H_
