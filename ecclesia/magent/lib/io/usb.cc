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

#include "absl/types/optional.h"
#include "ecclesia/lib/logging/logging.h"

namespace ecclesia {

UsbPortSequence::UsbPortSequence(absl::Span<const UsbPort> ports) {
  if (ports.size() > kDeviceChainMaxLength) {
    WarningLog() << "Usb ports excceed max length";
  }

  ports_.clear();
  std::copy(ports.begin(),
            ports.begin() +
                std::min(kDeviceChainMaxLength, static_cast<int>(ports.size())),
            std::back_inserter(ports_));
}

absl::optional<UsbPortSequence> UsbPortSequence::TryMake(
    absl::Span<const int> ports) {
  if (ports.size() > kDeviceChainMaxLength) {
    WarningLog() << "Usb ports excceed max length";
    return absl::nullopt;
  }

  std::vector<UsbPort> port_sequence;
  for (const auto port : ports) {
    auto maybe_port = UsbPort::TryMake(port);
    if (!maybe_port.has_value()) {
      return absl::nullopt;
    }
    port_sequence.push_back(maybe_port.value());
  }

  return UsbPortSequence(std::move(port_sequence));
}

int UsbPortSequence::size() const { return ports_.size(); }

absl::optional<UsbPortSequence> UsbPortSequence::Downstream(
    UsbPort port) const {
  if (size() == kDeviceChainMaxLength) {
    return absl::nullopt;
  }

  // The downstream sequence is this sequence with the port number appended.
  UsbPortSequence child(*this);
  child.ports_.push_back(port);
  return child;
}

bool operator==(const UsbPortSequence &lhs, const UsbPortSequence &rhs) {
  return std::equal(lhs.ports_.begin(), lhs.ports_.begin() + lhs.size(),
                    rhs.ports_.begin());
}

bool operator!=(const UsbPortSequence &lhs, const UsbPortSequence &rhs) {
  return !(lhs == rhs);
}

}  // namespace ecclesia
