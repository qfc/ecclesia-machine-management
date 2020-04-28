#include "magent/lib/io/smbus.h"

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"

namespace ecclesia {
namespace {

// Test device location comparator.
TEST(SmbusSmbusLocationTest, TestComparator) {
  EXPECT_GE(SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 0}}),
            SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 0}}));

  EXPECT_GE(SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 1}}),
            SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 0}}));

  EXPECT_GE(SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 0}}),
            SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 0}}));

  EXPECT_LT(SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 0}}),
            SmbusLocation({.bus = {.loc = 1}, .address = {.addr = 0}}));

  EXPECT_LT(SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 0}}),
            SmbusLocation({.bus = {.loc = 0}, .address = {.addr = 1}}));
}

}  // namespace
}  // namespace ecclesia
