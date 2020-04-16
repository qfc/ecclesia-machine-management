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

#ifndef ECCLESIA_LIB_MCEDECODER_BIT_OPERATOR_H_
#define ECCLESIA_LIB_MCEDECODER_BIT_OPERATOR_H_

#include <cassert>
#include <type_traits>
#include <utility>

namespace mcedecoder {

// A pair of int representing the end and start location of the bit range. The
// first value (end) should not be smaller than the second value (start).
using BitRange = std::pair<unsigned int, unsigned int>;

inline BitRange Bit(unsigned int location) {
  return BitRange(location, location);
}

inline BitRange Bits(unsigned int high, unsigned int low) {
  return BitRange(high, low);
}

// Extracts a range of bits from a value. For example, ExtractBits(0x12345678,
// BitRange(15, 8)) will return 0x56.
template <typename T>
inline T ExtractBits(const T &value, const BitRange &range) {
  assert(range.first >= range.second);
  assert(range.first < sizeof(T) * 8);
  using unsigned_T = typename std::make_unsigned<T>::type;
  unsigned_T unsigned_value = value;
  // Full mask (all 1s) in case the mask covers the full range of T
  unsigned_T mask = -1;
  int range_span = range.first - range.second + 1;
  if (range_span < sizeof(T) * 8) {
    // Partial mask whose lower (range.first - range.second + 1) bits are all 1s
    mask = (1 << range_span) - 1;
  }

  return (unsigned_value >> range.second) & mask;
}
}  // namespace mcedecoder
#endif  // ECCLESIA_LIB_MCEDECODER_BIT_OPERATOR_H_
