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

#include "ecclesia/magent/lib/eeprom/smbus_eeprom.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/magent/lib/eeprom/eeprom.h"
#include "ecclesia/magent/lib/io/smbus.h"
#include "ecclesia/magent/lib/io/smbus_mocks.h"

namespace ecclesia {
namespace {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrictMock;

TEST(SmbusEepromDeviceTest, Methods) {
  StrictMock<MockSmbusAccessInterface> access;
  auto loc = SmbusLocation::Make<37, 0x55>();
  SmbusDevice device(loc, &access);

  EXPECT_CALL(access, SendByte(_, _)).WillOnce(Return(absl::OkStatus()));

  EXPECT_CALL(access, ReceiveByte(_, _)).WillOnce(Return(absl::OkStatus()));

  EXPECT_CALL(access, ReadBlockI2C(_, _, _, _))
      .WillOnce(DoAll(SetArgPointee<3>(4), Return(absl::OkStatus())));

  EXPECT_CALL(access, WriteBlockI2C(_, _, _))
      .WillOnce(Return(absl::OkStatus()));

  EXPECT_CALL(access, Read8(_, _, _))
      .WillRepeatedly(DoAll(SetArgPointee<2>(1), Return(absl::OkStatus())));

  EXPECT_CALL(access, Read16(_, _, _)).WillRepeatedly(Return(absl::OkStatus()));

  EXPECT_CALL(access, Write8(_, _, _)).WillRepeatedly(Return(absl::OkStatus()));

  EXPECT_CALL(access, Write16(_, _, _))
      .WillRepeatedly(Return(absl::OkStatus()));

  // Create test target
  Eeprom::SizeType eeprom_size = {.type = ecclesia::Eeprom::SizeType::kFixed,
                                  .size = 8 * 1024};
  Eeprom::ModeType eeprom_mode = {.readable = 1, .writable = 1};

  ecclesia::SmbusEeprom2ByteAddr::Option motherboard_eeprom_option{
      .name = "motherboard",
      .size = eeprom_size,
      .mode = eeprom_mode,
      .device = device};

  SmbusEeprom2ByteAddr eeprom(motherboard_eeprom_option);

  auto sz = eeprom.GetSize();
  ASSERT_EQ(eeprom_size.size, sz.size);
  ASSERT_EQ(eeprom_size.type, sz.type);

  auto md = eeprom.GetMode();
  ASSERT_EQ(eeprom_mode.readable, md.readable);
  ASSERT_EQ(eeprom_mode.writable, md.writable);

  size_t len;
  std::vector<unsigned char> data1(6);

  EXPECT_TRUE(
      device.ReadBlockI2C(0, absl::Span<unsigned char>(data1.data(), 4), &len)
          .ok());
  EXPECT_EQ(len, 4);

  std::vector<unsigned char> data2(6);
  EXPECT_TRUE(
      device.WriteBlockI2C(0, absl::Span<const unsigned char>(data2.data(), 6))
          .ok());

  uint8_t data8;
  EXPECT_TRUE(device.SendByte(6).ok());
  EXPECT_TRUE(device.ReceiveByte(&data8).ok());
  EXPECT_TRUE(device.Read8(0, &data8).ok());

  uint16_t data16;
  EXPECT_TRUE(device.Read16(0, &data16).ok());

  EXPECT_TRUE(device.Write8(0, 6).ok());
  EXPECT_TRUE(device.Write16(0, 16).ok());
}

constexpr SmbusLocation loc = SmbusLocation::Make<37, 0x55>();
constexpr Eeprom::SizeType eeprom_size{
    .type = ecclesia::Eeprom::SizeType::kFixed, .size = 8 * 1024};
constexpr Eeprom::ModeType eeprom_mode{.readable = 1, .writable = 1};

class SmbusEepromTest : public ::testing::Test {
 public:
  SmbusEepromTest()
      : device_(SmbusDevice(loc, &access_)),
        motherboard_eeprom_option_({.name = "motherboard",
                                    .size = eeprom_size,
                                    .mode = eeprom_mode,
                                    .device = device_}),
        eeprom_(motherboard_eeprom_option_) {}

 protected:
  StrictMock<MockSmbusAccessInterface> access_;
  SmbusDevice device_;
  SmbusEeprom2ByteAddr::Option motherboard_eeprom_option_;
  SmbusEeprom2ByteAddr eeprom_;
};

TEST_F(SmbusEepromTest, DeviceRead8Success) {
  uint8_t expected = 0xbe;
  EXPECT_CALL(access_, Read8(_, _, _)).WillOnce(SmbusRead8(&expected));

  uint8_t data;
  EXPECT_TRUE(device_.Read8(0, &data).ok());
  EXPECT_EQ(data, expected);
}

TEST_F(SmbusEepromTest, DeviceRead16Success) {
  uint16_t expected = 0xbeef;
  EXPECT_CALL(access_, Read16(_, _, _)).WillOnce(SmbusRead16(&expected));

  uint16_t data;
  EXPECT_TRUE(device_.Read16(0, &data).ok());
  EXPECT_EQ(data, expected);
}

TEST_F(SmbusEepromTest, ReadBlockI2cSuccess) {
  std::vector<unsigned char> expected{1, 2, 3, 4, 5, 6, 7, 8};
  EXPECT_CALL(access_, ReadBlockI2C(_, _, _, _))
      .WillOnce(SmbusReadBlock(expected.data(), expected.size()));

  size_t len;
  std::vector<unsigned char> data(expected.size());
  EXPECT_TRUE(device_.ReadBlockI2C(0, absl::MakeSpan(data), &len).ok());
  EXPECT_EQ(len, expected.size());
  EXPECT_EQ(data, expected);
}

TEST_F(SmbusEepromTest, DeviceReceiveByteSuccess) {
  uint8_t expected = 0xbe;
  EXPECT_CALL(access_, ReceiveByte(_, _)).WillOnce(SmbusReceiveByte(&expected));

  uint8_t data;
  EXPECT_TRUE(device_.ReceiveByte(&data).ok());
  EXPECT_EQ(data, expected);
}

TEST_F(SmbusEepromTest, Eeprom2ByteAddrReadBytesSuccess) {
  uint8_t value = 0xbe;
  EXPECT_CALL(access_, ReceiveByte(_, _))
      .WillRepeatedly(SmbusReceiveByte(&value));
  EXPECT_CALL(access_, Write8(_, _, _))
      .WillRepeatedly(Return(absl::OkStatus()));

  std::vector<unsigned char> expected(8, 0xbe);
  std::vector<unsigned char> data(8);

  EXPECT_TRUE(eeprom_.ReadBytes(0x55, absl::MakeSpan(data)).has_value());
  EXPECT_EQ(data, expected);
}

TEST_F(SmbusEepromTest, Eeprom2ByteAddrReadBytesFail) {
  uint8_t value = 0xbe;
  EXPECT_CALL(access_, ReceiveByte(_, _))
      .WillRepeatedly(SmbusReceiveByte(&value));
  EXPECT_CALL(access_, Write8(_, _, _))
      .WillRepeatedly(Return(absl::InternalError("")));

  std::vector<unsigned char> expected(8, 0xbe);
  std::vector<unsigned char> data(8);

  EXPECT_FALSE(eeprom_.ReadBytes(0x55, absl::MakeSpan(data)).has_value());
}

TEST_F(SmbusEepromTest, Eeprom2ByteAddrReadBytesFail2) {
  EXPECT_CALL(access_, ReceiveByte(_, _))
      .WillRepeatedly(Return(absl::InternalError("")));
  EXPECT_CALL(access_, Write8(_, _, _))
      .WillRepeatedly(Return(absl::OkStatus()));

  std::vector<unsigned char> expected(8, 0xbe);
  std::vector<unsigned char> data(8);

  EXPECT_FALSE(eeprom_.ReadBytes(0x55, absl::MakeSpan(data)).has_value());
}

TEST_F(SmbusEepromTest, Eeprom2ByteAddrWriteBytesNullOpt) {
  std::vector<unsigned char> data(8, 0xbe);

  EXPECT_FALSE(eeprom_.WriteBytes(0x55, absl::MakeConstSpan(data)).has_value());
}

}  // namespace
}  // namespace ecclesia
