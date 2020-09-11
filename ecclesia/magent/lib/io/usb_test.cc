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

#include "gtest/gtest.h"
#include "absl/types/optional.h"

namespace ecclesia {
namespace {

TEST(UsbPortSequenceTest, ValidateStaticFunctions) {
  static constexpr UsbPortSequence kEmptyDefault;
  EXPECT_EQ(kEmptyDefault.Size(), 0);

  static constexpr UsbPortSequence kEmptyExplicit = UsbPortSequence::Make<>();
  EXPECT_EQ(kEmptyExplicit.Size(), 0);

  static constexpr UsbPortSequence kLongSequence =
      UsbPortSequence::Make<1, 2, 3, 4, 5>();
  EXPECT_EQ(kLongSequence.Size(), 5);

  static constexpr UsbPortSequence kMaxSequence =
      UsbPortSequence::Make<1, 2, 3, 4, 5, 6>();
  EXPECT_EQ(kMaxSequence.Size(), 6);

  auto maybe_long_to_max = kLongSequence.Downstream(UsbPort::Make<6>());
  ASSERT_TRUE(maybe_long_to_max.has_value());
  auto &long_to_max = maybe_long_to_max.value();
  EXPECT_EQ(kMaxSequence, long_to_max);
}

TEST(UsbPortSequenceTest, ValidateDynamicFunctions) {
  auto maybe_seq = UsbPortSequence::TryMake({1, 2, 3, 4, 5, 6, 7});
  EXPECT_FALSE(maybe_seq.has_value());

  maybe_seq = UsbPortSequence::TryMake({1, 2, 3, 4, 5});
  ASSERT_TRUE(maybe_seq.has_value());
  auto &seq_0 = maybe_seq.value();
  EXPECT_EQ(seq_0.Size(), 5);

  auto maybe_seq_1 = UsbPortSequence::TryMake({1, 2, 3, 4, 5, 6});
  ASSERT_TRUE(maybe_seq_1.has_value());
  auto &seq_1 = maybe_seq_1.value();
  auto seq_0_child = seq_0.Downstream(UsbPort::Make<6>());
  ASSERT_TRUE(seq_0_child.has_value());
  EXPECT_EQ(seq_0_child.value(), seq_1);

  auto maybe_seq_1_child = seq_1.Downstream(UsbPort::Make<7>());
  EXPECT_FALSE(maybe_seq_1_child.has_value());
}

TEST(UsbLocationTest, ValidateStaticFunctions) {
  static constexpr UsbLocation kController = UsbLocation::Make<3>();
  EXPECT_EQ(kController.Bus().value(), 3);
  EXPECT_EQ(kController.NumPorts(), 0);

  static constexpr UsbLocation kDevice = UsbLocation::Make<3, 1, 1, 5>();
  EXPECT_EQ(kDevice.Bus().value(), 3);
  EXPECT_EQ(kDevice.NumPorts(), 3);
}

TEST(GetUsbPluginIdWithSignatureTest, GetCorrectId) {
  UsbSignature sleipnir_signature{0x18d1, 0x0215};
  EXPECT_EQ(GetUsbPluginIdWithSignature(sleipnir_signature),
            UsbPluginId::kSleipnirBmc);

  UsbSignature non_exist_signature{0x1234, 0x5678};
  EXPECT_EQ(GetUsbPluginIdWithSignature(non_exist_signature),
            UsbPluginId::kUnknown);
}

TEST(GetUsbSignatureWithPluginIdTest, GetCorrectSignature) {
  auto maybe_signature = GetUsbSignatureWithPluginId(UsbPluginId::kSleipnirBmc);
  ASSERT_TRUE(maybe_signature.ok());
  EXPECT_EQ(maybe_signature.value(), (UsbSignature{0x18d1, 0x0215}));

  maybe_signature = GetUsbSignatureWithPluginId(UsbPluginId::kUnknown);
  EXPECT_FALSE(maybe_signature.ok());
}

}  // namespace
}  // namespace ecclesia
