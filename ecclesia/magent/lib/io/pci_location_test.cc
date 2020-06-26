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

#include "ecclesia/magent/lib/io/pci_location.h"

#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "absl/types/optional.h"

namespace ecclesia {
namespace {

TEST(PciLocationTest, VerifyIdentityOps) {
  // domain has range [0-65535], we only test [0-8] here.
  for (int domain = 0; domain <= 0x8; ++domain) {
    for (int bus = 0; bus <= 0xff; ++bus) {
      for (int device = 0; device <= 0x1f; ++device) {
        for (int function = 0; function <= 7; ++function) {
          auto maybe_loc = PciLocation::TryMake(domain, bus, device, function);
          ASSERT_TRUE(maybe_loc.has_value());
          std::string loc_str =
              absl::StrFormat("%s", absl::FormatStreamed(maybe_loc.value()));
          ASSERT_EQ(12, loc_str.length());

          auto loc_from_str = PciLocation::FromString(loc_str);
          ASSERT_TRUE(loc_from_str.has_value());

          EXPECT_EQ(maybe_loc.value(), loc_from_str.value());
        }
      }
    }
  }
}

TEST(PciLocationTest, PciLocationFailTests) {
  // from empty string
  auto loc = PciLocation::FromString("");
  EXPECT_EQ(loc, absl::nullopt);

  // short form not parsable.
  loc = PciLocation::FromString("02:01.0");
  EXPECT_EQ(loc, absl::nullopt);

  // Strict format: 0001:02:03.4, 0 padding is enforced.
  loc = PciLocation::FromString("0001:2:3.4");
  EXPECT_EQ(loc, absl::nullopt);

  // Invalid character.
  loc = PciLocation::FromString("0001:02:0g.7");
  EXPECT_EQ(loc, absl::nullopt);

  // Function number out of range.
  loc = PciLocation::FromString("0001:02:03.9");
  EXPECT_EQ(loc, absl::nullopt);

  // TryMake: Function number out of range.
  loc = PciLocation::TryMake(1, 2, 3, 9);
  EXPECT_EQ(loc, absl::nullopt);
}

TEST(PciLocationTest, TestComparator) {
  EXPECT_GE((PciLocation::Make<0, 0, 0, 0>()),
            (PciLocation::Make<0, 0, 0, 0>()));

  EXPECT_GE((PciLocation::Make<0, 1, 2, 3>()),
            (PciLocation::Make<0, 0, 0, 0>()));

  EXPECT_GE((PciLocation::Make<1, 0, 0, 0>()),
            (PciLocation::Make<0, 0xff, 0x1f, 0x7>()));

  EXPECT_LT((PciLocation::Make<0, 0xff, 0x1f, 0x7>()),
            (PciLocation::Make<1, 0, 0, 0>()));

  EXPECT_LT((PciLocation::Make<0, 0, 0, 0>()),
            (PciLocation::Make<0, 1, 2, 3>()));

  EXPECT_EQ((PciLocation::Make<2, 4, 5, 6>()),
            (PciLocation::Make<2, 4, 5, 6>()));
}

TEST(PciLocationTest, IsHashable) {
  absl::flat_hash_map<PciLocation, std::string> pci_map;

  // Push different values into the map.
  auto loc0 = PciLocation::Make<1, 2, 3, 4>();
  auto loc1 = PciLocation::Make<0, 2, 3, 4>();
  auto loc2 = PciLocation::Make<1, 0, 3, 4>();
  auto loc3 = PciLocation::Make<1, 2, 0, 4>();
  auto loc4 = PciLocation::Make<1, 2, 3, 0>();

  pci_map[loc0] = absl::StrFormat("%s", absl::FormatStreamed(loc0));
  pci_map[loc1] = absl::StrFormat("%s", absl::FormatStreamed(loc1));
  pci_map[loc2] = absl::StrFormat("%s", absl::FormatStreamed(loc2));
  pci_map[loc3] = absl::StrFormat("%s", absl::FormatStreamed(loc3));
  pci_map[loc4] = absl::StrFormat("%s", absl::FormatStreamed(loc4));

  EXPECT_EQ(pci_map.size(), 5);

  auto iter = pci_map.find(PciLocation::Make<1, 0, 3, 4>());
  ASSERT_NE(iter, pci_map.end());
  EXPECT_EQ(iter->first, loc2);
  EXPECT_EQ(iter->second, "0001:00:03.4");
}

}  // namespace
}  // namespace ecclesia
