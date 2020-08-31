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
  EXPECT_EQ(maybe_seq.value().size(), 5);
}

}  // namespace
}  // namespace ecclesia
