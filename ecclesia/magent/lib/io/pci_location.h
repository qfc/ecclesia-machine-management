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

// Define PciLocaiton type, provides templated function Make() which does range
// check at compile time, and TryMake() function that tries to construct a
// ranged-checked PciLocation at run time.

#ifndef ECCLESIA_MAGENT_LIB_IO_PCI_LOCATION_H_
#define ECCLESIA_MAGENT_LIB_IO_PCI_LOCATION_H_

#include <string>

#include "absl/strings/numbers.h"
#include "absl/strings/str_format.h"
#include "absl/types/optional.h"
#include "lib/types/fixed_range_int.h"

namespace ecclesia {

// Each domain contains buses. [0-65535]
class PciDomain : public FixedRangeInteger<PciDomain, int, 0, 65535> {
 public:
  explicit constexpr PciDomain(BaseType value) : BaseType(value) {}
};

// Number of the bus the device is attached to. [0-255]
class PciBusNum : public FixedRangeInteger<PciBusNum, int, 0, 255> {
 public:
  explicit constexpr PciBusNum(BaseType value) : BaseType(value) {}
};

// Device ID of the device on the PCI bus.  [0-31]
class PciDeviceNum : public FixedRangeInteger<PciDeviceNum, int, 0, 31> {
 public:
  explicit constexpr PciDeviceNum(BaseType value) : BaseType(value) {}
};

// Function of the device (a single physical device can expose multiple
// functions).  [0-7]
class PciFunctionNum : public FixedRangeInteger<PciFunctionNum, int, 0, 7> {
 public:
  explicit constexpr PciFunctionNum(BaseType value) : BaseType(value) {}
};

// An identifier representing a PciLocation.
class PciLocation {
 public:
  constexpr PciLocation(PciDomain domain, PciBusNum bus, PciDeviceNum device,
                        PciFunctionNum function)
      : domain_(domain), bus_(bus), device_(device), function_(function) {}

  PciLocation(const PciLocation &) = default;
  PciLocation &operator=(const PciLocation &) = default;

  // Create a PciLocation whose range is statically checked at compile time.
  template <int domain, int bus, int device, int function>
  static constexpr PciLocation Make() {
    return PciLocation(PciDomain::Make<domain>(), PciBusNum::Make<bus>(),
                       PciDeviceNum::Make<device>(),
                       PciFunctionNum::Make<function>());
  }

  // Create a PciLocation whose range is checked at run time.
  static absl::optional<PciLocation> TryMake(int domain, int bus, int device,
                                             int function) {
    auto maybe_domain = PciDomain::TryMake(domain);
    auto maybe_bus = PciBusNum::TryMake(bus);
    auto maybe_device = PciDeviceNum::TryMake(device);
    auto maybe_function = PciFunctionNum::TryMake(function);

    if (!maybe_domain.has_value() || !maybe_bus.has_value() ||
        !maybe_device.has_value() || !maybe_function.has_value()) {
      return absl::nullopt;
    }

    return PciLocation(maybe_domain.value(), maybe_bus.value(),
                       maybe_device.value(), maybe_function.value());
  }

  PciDomain domain() { return domain_; }
  PciBusNum bus() { return bus_; }
  PciDeviceNum device() { return device_; }
  PciFunctionNum function() { return function_; }

  // Convert a string to PciLocation in runtime.
  // string format: 0000:17:08.2
  static absl::optional<PciLocation> FromString(absl::string_view dev_str);

  // PciLocation relational operators.
  // Order is equivalent to that of a <domain, bus, device, function> tuple.
  friend bool operator==(const PciLocation &lhs, const PciLocation &rhs) {
    return std::tuple(lhs.domain_.value(), lhs.bus_.value(),
                      lhs.device_.value(), lhs.function_.value()) ==
           std::tuple(rhs.domain_.value(), rhs.bus_.value(),
                      rhs.device_.value(), rhs.function_.value());
  }
  friend bool operator!=(const PciLocation &lhs, const PciLocation &rhs) {
    return !(lhs == rhs);
  }
  friend bool operator<(const PciLocation &lhs, const PciLocation &rhs) {
    return std::tuple(lhs.domain_.value(), lhs.bus_.value(),
                      lhs.device_.value(), lhs.function_.value()) <
           std::tuple(rhs.domain_.value(), rhs.bus_.value(),
                      rhs.device_.value(), rhs.function_.value());
  }
  friend bool operator>(const PciLocation &lhs, const PciLocation &rhs) {
    return (rhs < lhs);
  }
  friend bool operator<=(const PciLocation &lhs, const PciLocation &rhs) {
    return !(rhs < lhs);
  }
  friend bool operator>=(const PciLocation &lhs, const PciLocation &rhs) {
    return !(lhs < rhs);
  }

  // Support hashing of locations for use as a key in hash maps.
  template <typename H>
  friend H AbslHashValue(H h, const PciLocation &loc) {
    return H::combine(std::move(h), loc.domain_.value(), loc.bus_.value(),
                      loc.device_.value(), loc.function_.value());
  }

  // String conversion. This deliberately follows the
  // domain:bus:device.function format that the kernel uses in sysfs.
  friend std::ostream &operator<<(std::ostream &os,
                                  const PciLocation &location) {
    return os << absl::StreamFormat(
               "%04x:%02x:%02x.%x", location.domain_.value(),
               location.bus_.value(), location.device_.value(),
               location.function_.value());
  }

 private:
  PciDomain domain_;
  PciBusNum bus_;
  PciDeviceNum device_;
  PciFunctionNum function_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_LOCATION_H_
