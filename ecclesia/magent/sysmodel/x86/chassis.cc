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

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/types/optional.h"
#include "ecclesia/lib/logging/logging.h"
#include "ecclesia/magent/lib/io/usb.h"
#include "ecclesia/magent/lib/io/usb_sysfs.h"

namespace ecclesia {

absl::optional<ChassisId> DetectChassisByUsb(
    UsbDiscoveryInterface *usb_discover_intf) {
  auto maybe_usb_devices = usb_discover_intf->EnumerateAllUsbDevices();
  if (!maybe_usb_devices.ok()) {
    ecclesia::ErrorLog() << "Failed to enumerate USB devices. status: "
                         << maybe_usb_devices.status();
  } else {
    for (auto &usb_device : maybe_usb_devices.value()) {
      if (auto maybe_signature = usb_device->GetSignature();
          maybe_signature.ok()) {
        UsbPluginId usb_id =
            GetUsbPluginIdWithSignature(maybe_signature.value());
        if (usb_id == UsbPluginId::kSleipnirBmc) {
          return ChassisId::kSleipnir;
        }
      }
    }
  }
  return absl::nullopt;
}

std::vector<ChassisId> CreateChassis(UsbDiscoveryInterface *usb_discover_intf) {
  std::vector<ChassisId> chassis_vec;
  // Hardcode the Indus Chassis for now.
  chassis_vec.push_back(ChassisId::kIndus);

  // Detect USB-based chassis and add the chassis if found.
  if (auto maybe_chassis = DetectChassisByUsb(usb_discover_intf);
      maybe_chassis.has_value()) {
    chassis_vec.push_back(maybe_chassis.value());
  }
  return chassis_vec;
}

}  // namespace ecclesia
