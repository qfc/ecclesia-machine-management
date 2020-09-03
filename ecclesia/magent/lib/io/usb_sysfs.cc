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

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "ecclesia/lib/apifs/apifs.h"
#include "ecclesia/lib/file/path.h"
#include "ecclesia/magent/lib/io/usb.h"
#include "re2/re2.h"

namespace ecclesia {
namespace {

// Convert a sysfs device directory name into a device location. Returns nullopt
// if the conversion failed.
//
// For every USB device connected to the system there will be a sysfs entry
// created based on what USB bus # the device lives on as well as the sequence
// of ports between the root controller and said device. The sysfs entry will
// follow the pattern $BUS-$PORT0.$PORT1.$PORT2...
//
// So for example a device connected directly into port 2 on root controller
// 3 will have an entry at 3-2, while a device connected into port 6 on a hub
// connected into port 3 on root controller 4 will have an entry at 4-3.6, and
// so on. Note that this sequence cannot go on indefinitely as a maximum chain
// length is set by the USB standard (6 for USB 3.0).
//
// There is one special case, the root controllers themselves. These each get
// a device entry at "usbX" where X is the number of the USB controller (i.e.
// the bus number). This is generally a symlink to the actual device entry in
// the PCI part of sysfs.
absl::optional<UsbLocation> DirectoryToDeviceLocation(
    absl::string_view dirname) {
  int bus;
  std::string port_substr;
  if (RE2::FullMatch(dirname, "usb(\\d+)", &bus)) {
    auto usb_bus = UsbBusLocation::TryMake(bus);
    if (!usb_bus.has_value()) {
      return absl::nullopt;
    }
    return UsbLocation(usb_bus.value(), UsbPortSequence());
  } else if (RE2::FullMatch(dirname, "(\\d+)-(\\d+(?:\\.\\d+)*)", &bus,
                            &port_substr)) {
    auto maybe_usb_bus = UsbBusLocation::TryMake(bus);
    if (!maybe_usb_bus.has_value()) {
      return absl::nullopt;
    }

    std::vector<int> ports;
    std::vector<absl::string_view> ports_str =
        absl::StrSplit(port_substr, '.', absl::SkipEmpty());
    UsbPortSequence seq;
    for (const auto &port_str : ports_str) {
      int port;
      if (absl::SimpleAtoi(port_str, &port)) {
        if (auto maybe_usb_port = UsbPort::TryMake(port);
            maybe_usb_port.has_value()) {
          if (auto maybe_seq = seq.Downstream(maybe_usb_port.value());
              maybe_seq.has_value()) {
            seq = maybe_seq.value();
          }
        }
      } else {
        return absl::nullopt;
      }
    }
    return UsbLocation(maybe_usb_bus.value(), seq);
  }
  return absl::nullopt;
}
}  // namespace

SysfsUsbDiscovery::SysfsUsbDiscovery(absl::string_view sysfs_dir)
    : sysfs_devices_dir_(absl::StrCat(sysfs_dir, "/bus/usb/devices")) {}

absl::Status SysfsUsbDiscovery::EnumerateAllUsbDevices(
    std::vector<UsbLocation> *devices) const {
  ApifsDirectory dir(sysfs_devices_dir_);

  auto maybe_usb_device_entries = dir.ListEntries();
  if (!maybe_usb_device_entries.ok()) {
    return maybe_usb_device_entries.status();
  }

  devices->clear();

  for (absl::string_view entry : *maybe_usb_device_entries) {
    auto maybe_usb_location = DirectoryToDeviceLocation(GetBasename(entry));

    if (maybe_usb_location.has_value()) {
      devices->push_back(maybe_usb_location.value());
    }
  }
  return absl::OkStatus();
}

}  // namespace ecclesia
