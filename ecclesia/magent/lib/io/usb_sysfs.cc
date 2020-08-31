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

#include "absl/strings/str_cat.h"

namespace ecclesia {
SysfsUsbDiscovery::SysfsUsbDiscovery(absl::string_view sysfs_dir)
    : sysfs_devices_dir_(absl::StrCat(sysfs_dir, "/bus/usb/devices")) {}

absl::Status SysfsUsbDiscovery::EnumerateAllUsbDevices(
    std::vector<UsbLocation> *devices) const {
  // ecclesia/lib/apifs
  return absl::OkStatus();
}

}  // namespace ecclesia
