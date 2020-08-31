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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace ecclesia {
namespace {

// Sanity test the UsbPortSequence behavior.
TEST(UsbHelpersTest, UsbPortSequenceValidation) {
  auto maybe_seq = UsbPortSequence::TryMake({1, 2, 3, 4, 5, 6, 7});
  EXPECT_FALSE(maybe_seq.has_value());

  maybe_seq = UsbPortSequence::TryMake({1, 2, 3, 4, 5});
  ASSERT_TRUE(maybe_seq.has_value());
  auto &seq_0 = maybe_seq.value();
  EXPECT_EQ(seq_0.size(), 5);

  auto maybe_seq_1 = UsbPortSequence::TryMake({1, 2, 3, 4, 5, 6});
  ASSERT_TRUE(maybe_seq_1.has_value());
  auto &seq_1 = maybe_seq_1.value();
  auto seq_0_child = seq_0.Downstream(UsbPort::Make<6>());
  ASSERT_TRUE(seq_0_child.has_value());
  EXPECT_EQ(seq_0_child.value(), seq_1);

  auto maybe_seq_1_child = seq_1.Downstream(UsbPort::Make<7>());
  EXPECT_FALSE(maybe_seq_1_child.has_value());
}

}  // namespace
}  // namespace ecclesia
