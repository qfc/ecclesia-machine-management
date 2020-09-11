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

#include "ecclesia/magent/lib/io/usb.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <tuple>

#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/lib/logging/globals.h"
#include "ecclesia/lib/logging/logging.h"
#include "ecclesia/lib/types/fixed_range_int.h"

namespace ecclesia {

absl::optional<UsbPortSequence> UsbPortSequence::TryMake(
    absl::Span<const int> ports) {
  if (ports.size() > kDeviceChainMaxLength) {
    WarningLog() << "Usb ports excceed max length";
    return absl::nullopt;
  }

  StoredArray stored_ports;
  for (size_t i = 0; i < ports.size(); ++i) {
    auto maybe_port = UsbPort::TryMake(ports[i]);
    if (!maybe_port.has_value()) {
      return absl::nullopt;
    }
    stored_ports[i].value = *maybe_port;
  }
  return UsbPortSequence(stored_ports, ports.size());
}

size_t UsbPortSequence::Size() const { return size_; }

absl::optional<UsbPort> UsbPortSequence::Port(size_t index) const {
  if (index >= size_) return absl::nullopt;
  return ports_[index].value;
}

absl::optional<UsbPortSequence> UsbPortSequence::Downstream(
    UsbPort port) const {
  if (size_ == kDeviceChainMaxLength) return absl::nullopt;

  // The downstream sequence is this sequence with the port number appended.
  StoredArray child_ports = ports_;
  child_ports[size_].value = port;
  return UsbPortSequence(child_ports, size_ + 1);
}

bool operator==(const UsbPortSequence &lhs, const UsbPortSequence &rhs) {
  if (lhs.size_ != rhs.size_) return false;
  return std::equal(
      lhs.ports_.begin(), lhs.ports_.begin() + lhs.size_, rhs.ports_.begin(),
      [](const auto &lhs, const auto &rhs) { return lhs.value == rhs.value; });
}

bool operator!=(const UsbPortSequence &lhs, const UsbPortSequence &rhs) {
  return !(lhs == rhs);
}

bool operator==(const UsbLocation &lhs, const UsbLocation &rhs) {
  return std::tie(lhs.bus_, lhs.ports_) == std::tie(rhs.bus_, rhs.ports_);
}

bool operator!=(const UsbLocation &lhs, const UsbLocation &rhs) {
  return !(lhs == rhs);
}

}  // namespace ecclesia
