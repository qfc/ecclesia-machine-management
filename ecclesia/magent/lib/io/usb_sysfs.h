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
#include "absl/strings/string_view.h"
#include "ecclesia/magent/lib/io/usb.h"

namespace ecclesia {

class SysfsUsbDiscovery : public UsbDiscoveryInterface {
 public:
  SysfsUsbDiscovery() : SysfsUsbDiscovery("/sys") {}
  // This constructor is for testing purpose.
  explicit SysfsUsbDiscovery(absl::string_view sysfs_dir);
  absl::Status EnumerateAllUsbDevices(
      std::vector<UsbLocation> *devices) const override;

 private:
  const std::string sysfs_devices_dir_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_USB_SYSFS_H_
