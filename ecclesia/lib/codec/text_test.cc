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

#include "ecclesia/lib/codec/text.h"

#include <cstdint>
#include <string>

#include "gtest/gtest.h"
#include "absl/status/status.h"

namespace ecclesia {
namespace {

TEST(TextDecode, DecodeValidBCD) {
  static constexpr char kBcdTranslated[] = "42-421 ...";
  static constexpr uint8_t kValidBcdString[] = {0x42, 0xB4, 0x21, 0xAC, 0xCC};

  std::string value;
  auto status = ParseBcdPlus(kValidBcdString, &value);
  ASSERT_TRUE(status.ok());
  EXPECT_EQ(kBcdTranslated, value);
}

TEST(TextDecode, DecodeValidSixBit) {
  static constexpr char k6BitTranslated[] = "WHY?";
  static constexpr char k6BitTranslated_2[] = "WHY?F";
  static constexpr uint8_t kValidSixBitAscii[] = {0x37, 0x9A, 0x7F};
  static constexpr uint8_t kValidSixBitAscii_2[] = {0x37, 0x9A, 0x7F, 0x66};

  std::string value;
  auto status = ParseSixBitAscii(kValidSixBitAscii, &value);
  ASSERT_TRUE(status.ok());
  EXPECT_EQ(value, std::string(k6BitTranslated));

  status = ParseSixBitAscii(kValidSixBitAscii_2, &value);
  ASSERT_TRUE(status.ok());
  EXPECT_EQ(value, std::string(k6BitTranslated_2));
}

}  // namespace
}  // namespace ecclesia
