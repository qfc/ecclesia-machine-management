// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lib/mcedecoder/bit_operator.h"

#include "base/integral_types.h"
#include "testing/base/public/gunit.h"

namespace mcedecoder {
namespace {

TEST(BitOperatorTest, ExtractCorrectBit) {
  uint32 value = 0x48;
  for (int i = 0; i < 32; ++i) {
    if (i == 3 || i == 6) {
      EXPECT_EQ(ExtractBits(value, Bit(i)), 1);
    } else {
      EXPECT_EQ(ExtractBits(value, Bit(i)), 0);
    }
  }
}

TEST(BitOperatorTest, ExtractCorrectBitRange) {
  uint32 value = 0x12345678;
  EXPECT_EQ(ExtractBits(value, Bits(15, 8)), 0x56);
  EXPECT_EQ(ExtractBits(value, Bits(27, 20)), 0x23);
  EXPECT_EQ(ExtractBits(value, Bits(31, 0)), value);
}

}  // namespace
}  // namespace mcedecoder
