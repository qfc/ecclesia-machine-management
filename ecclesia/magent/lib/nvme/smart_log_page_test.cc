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

#include "ecclesia/magent/lib/nvme/smart_log_page.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"

namespace ecclesia {

// SmartLogPage data collected from NVMe SSD, nvme-cli reports:
// -----------------------------------------------
// critical_warning                    : 0
// temperature                         : 41 C / 314 K
// available_spare                     : 98%
// available_spare_threshold           : 10%
// percentage_used                     : 0%
// data_units_read                     : 1,044
// data_units_written                  : 15
// host_read_commands                  : 1,027,083
// host_write_commands                 : 1,905
// controller_busy_time                : 0
// power_cycles                        : 4
// power_on_hours                      : 408
// unsafe_shutdowns                    : 1
// media_errors                        : 0
// num_err_log_entries                 : 0
// Warning Temperature Time            : 0
// Critical Composite Temperature Time : 0
// Thermal Management T1 Trans Count   : 0
// Thermal Management T2 Trans Count   : 0
// Thermal Management T1 Total Time    : 0
// Thermal Management T2 Total Time    : 0
constexpr unsigned char kSmartLogData[] = {
    /* 0x0000 */ 0x00, 0x3a, 0x01, 0x62, 0x0a, 0x00, 0x00, 0x00,
    /* 0x0008 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0010 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0018 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0020 */ 0x14, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0028 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0030 */ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0038 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0040 */ 0x0b, 0xac, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0048 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0050 */ 0x71, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0058 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0060 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0068 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0070 */ 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0078 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0080 */ 0x98, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0088 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0090 */ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0098 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00a0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00a8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00b8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00c0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00c8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00d8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00e0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00e8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00f0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x00f8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0100 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0108 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0110 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0118 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0128 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0130 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0138 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0140 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0148 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0150 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0158 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0160 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0168 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0170 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0178 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0180 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0188 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0190 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x0198 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01a0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01a8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01b8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01c0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01c8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01d0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01d8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01e0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01e8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01f0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 0x01f8 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

TEST(SmartLogPageTest, ParseSmartData) {
  std::unique_ptr<SmartLogPageInterface> smart(SmartLogPage::Parse(std::string(
      kSmartLogData,
      kSmartLogData + sizeof(kSmartLogData) / sizeof(kSmartLogData[0]))));

  ASSERT_TRUE(smart.get());

  EXPECT_EQ(0, smart->critical_warning());
  EXPECT_EQ(314, smart->composite_temperature_kelvins());
  EXPECT_EQ(98, smart->available_spare());
  EXPECT_EQ(10, smart->available_spare_threshold());
  EXPECT_EQ(1044, smart->data_units_read());
  EXPECT_EQ(15, smart->data_units_written());
  EXPECT_EQ(1027083, smart->host_reads());
  EXPECT_EQ(1905, smart->host_writes());
  EXPECT_EQ(0, smart->controller_busy_time_minutes());
  EXPECT_EQ(4, smart->power_cycles());
  EXPECT_EQ(408, smart->power_on_hours());
  EXPECT_EQ(1, smart->unsafe_shutdowns());
}

}  // namespace ecclesia
