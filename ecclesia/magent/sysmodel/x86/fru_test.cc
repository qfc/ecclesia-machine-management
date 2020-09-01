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

#include "ecclesia/magent/sysmodel/x86/fru.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ecclesia/magent/lib/ipmi/ipmi_mock.h"

namespace ecclesia {
namespace {

using ::testing::_;
using ::testing::Return;

TEST(IpmiSysmodelFruReaderTest, ReadFailure) {
  MockIpmiInterface ipmi_intf;
  uint16_t fru_id = 1;
  IpmiSysmodelFruReader ipmi_fru_reader(&ipmi_intf, fru_id);
  EXPECT_CALL(ipmi_intf, ReadFru(fru_id, 0, _))
      .WillOnce(Return(absl::NotFoundError("not found!")));
  EXPECT_FALSE(ipmi_fru_reader.Read());
}

TEST(IpmiSysmodelFruReaderTest, ReadFruSucces) {
  MockIpmiInterface ipmi_intf;
  uint16_t fru_id = 1;
  IpmiSysmodelFruReader ipmi_fru_reader(&ipmi_intf, fru_id);
  // This is the data collected from a real Sleipnir BMC via IPMI.
  std::vector<uint8_t> data = {
      1,  0,   0,  1,   0,   0,   0,   254, 1,   8,   0,   58,  134, 189, 198,
      81, 117, 97, 110, 116, 97,  204, 83,  108, 101, 105, 112, 110, 105, 114,
      32, 66,  77, 67,  207, 83,  82,  67,  81,  84,  87,  49,  57,  51,  49,
      48, 48,  49, 50,  53,  202, 49,  48,  53,  51,  57,  52,  56,  45,  48,
      50, 0,   0,  193, 0,   0,   0,   0,   0,   0,   0,   69};
  auto expect_data = absl::MakeSpan(data);
  EXPECT_CALL(ipmi_intf, ReadFru(fru_id, 0, _))
      .WillOnce(IpmiReadFru(expect_data.data()));
  auto optional_fru_reader = ipmi_fru_reader.Read();
  ASSERT_TRUE(optional_fru_reader.has_value());
  auto fru_reader = optional_fru_reader.value();
  EXPECT_EQ(fru_reader.GetManufacturer(), "Quanta");
  EXPECT_EQ(fru_reader.GetPartNumber(), "1053948-02");
  EXPECT_EQ(fru_reader.GetSerialNumber(), "SRCQTW193100125");
}

}  // namespace
}  // namespace ecclesia
