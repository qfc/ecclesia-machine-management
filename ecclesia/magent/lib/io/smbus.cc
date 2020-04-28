#include "magent/lib/io/smbus.h"

#include "absl/strings/str_format.h"

namespace ecclesia {

std::ostream &operator<<(std::ostream &os, const SmbusLocation &location) {
  return os << absl::StrFormat("%d-%02x", location.bus.loc,
                               location.address.addr);
}

bool operator==(const SmbusLocation &lhs, const SmbusLocation &rhs) {
  return std::tie(lhs.bus.loc, lhs.address.addr) ==
         std::tie(rhs.bus.loc, rhs.address.addr);
}

bool operator<(const SmbusLocation &lhs, const SmbusLocation &rhs) {
  return std::tie(lhs.bus.loc, lhs.address.addr) <
         std::tie(rhs.bus.loc, rhs.address.addr);
}

}  // namespace ecclesia
