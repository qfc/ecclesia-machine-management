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

#include "ecclesia/magent/lib/nvme/device_self_test_log.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "ecclesia/magent/lib/nvme/nvme_types.h"

namespace ecclesia {
namespace {

// output from nvme_cli

//  Current operation  : 0
//  Current Completion : 0%
//  Self Test Result[0]:
//    Operation Result             : 0
//    Self Test Code               : 2
//    Valid Diagnostic Information : 0
//    Power on hours (POH)         : 0x5f
//    Vendor Specific              : 0 0
//  Self Test Result[1]:
//    Operation Result             : 0
//    Self Test Code               : 2
//    Valid Diagnostic Information : 0
//    Power on hours (POH)         : 0x5f
//    Vendor Specific              : 0 0
//  Self Test Result[2]:
//    Operation Result             : 0
//    Self Test Code               : 1
//    Valid Diagnostic Information : 0
//    Power on hours (POH)         : 0x5f
//    Vendor Specific              : 0 0

// Contents of the array are obtained from the output of nvme_cli
// Example command: ./nvme get-log /dev/nvme0 -n 0 -i 6 -l 564
constexpr unsigned char kDeviceSelfTestLogData[kDeviceSelfTestLogFormatSize] = {
    /*0000*/ 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    /*0008*/ 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0010*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0018*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0020*/ 0x20, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00,
    /*0028*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0030*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0038*/ 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    /*0040*/ 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0048*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0050*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0058*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0060*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0068*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0070*/ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    /*0078*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0080*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0088*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0090*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0098*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00a0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00a8*/ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    /*00b0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00b8*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00c0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00c8*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00d0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00d8*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00e0*/ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    /*00e8*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00f0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*00f8*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0100*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0108*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0110*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0118*/ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    /*0120*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0128*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0130*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0138*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0140*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0148*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0150*/ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    /*0158*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0160*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0168*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0170*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0178*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0180*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0188*/ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    /*0190*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0198*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01a0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01a8*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01b0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01b8*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01c0*/ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    /*01c8*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01d0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01d8*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01e0*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01e0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01f0*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*01f8*/ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
    /*0200*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0208*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0210*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0218*/ 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0220*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0228*/ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*0230*/ 0x00, 0x00, 0x00, 0x00,
};

TEST(DeviceSelfTestLogTest, ParseDeviceSelfTestLog) {
  auto ret = DeviceSelfTestLog::Parse(
      std::string(kDeviceSelfTestLogData,
                  kDeviceSelfTestLogData + kDeviceSelfTestLogFormatSize));
  ASSERT_TRUE(ret.ok());
  std::unique_ptr<DeviceSelfTestLog> self_test_log = std::move(ret.value());

  ASSERT_NE(self_test_log.get(), nullptr);
  EXPECT_EQ(self_test_log->CurrentSelfTestStatus(),
            DeviceSelfTestLog::CurrentSelfTestStatusResult::kNoTestInProgress);
  std::vector<DeviceSelfTestResult> results =
      self_test_log->CompletedSelfTests();
  ASSERT_EQ(results.size(), 3);

  for (int i = 0; i < results.size(); ++i) {
    SCOPED_TRACE(absl::StrFormat("Failure in comparing result number %d", i));
    if (i == 2) {
      EXPECT_EQ(results[i].DeviceSelfTestCode(),
                DeviceSelfTestResult::kShortTest);
    } else {
      EXPECT_EQ(results[i].DeviceSelfTestCode(),
                DeviceSelfTestResult::kExtendedTest);
    }
    EXPECT_EQ(results[i].SelfTestResult(), DeviceSelfTestResult::kSuccess);
    EXPECT_EQ(results[i].FailedSegmentNumber(), absl::nullopt);
    EXPECT_EQ(results[i].PowerOnHours(), 0x5f);
    EXPECT_EQ(results[i].FailingNamespace(), absl::nullopt);
    EXPECT_EQ(results[i].FailingLBA(), absl::nullopt);
    EXPECT_EQ(results[i].StatusCodeType(), absl::nullopt);
    EXPECT_EQ(results[i].StatusCode(), absl::nullopt);
    EXPECT_EQ(results[i].VendorSpecificField(), 0x0);
  }
}

}  // namespace
}  // namespace ecclesia
