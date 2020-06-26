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

#include "ecclesia/lib/io/msr.h"

#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "ecclesia/lib/file/test_filesystem.h"

namespace ecclesia {
namespace {

class MsrTest : public ::testing::Test {
 protected:
  MsrTest()
      : fs_(GetTestTempdirPath()), msr_(GetTestTempdirPath("dev/cpu/0/msr")) {
    fs_.CreateDir("/dev/cpu/0");

    // content: 0a 0a 0a 0a 0a 0a 0a 0a 0a 0a
    fs_.CreateFile("/dev/cpu/0/msr", "\n\n\n\n\n\n\n\n\n\n");
  }

  TestFilesystem fs_;
  Msr msr_;
};

TEST(Msr, TestMsrNotExist) {
  uint64_t out_msr_data;
  Msr msr_non_exist(GetTestTempdirPath("dev/cpu/400/msr"));
  EXPECT_FALSE(msr_non_exist.Read(2, &out_msr_data).ok());

  EXPECT_FALSE(msr_non_exist.Write(1, 0xDEADBEEFDEADBEEF).ok());
}

TEST_F(MsrTest, TestReadWriteOk) {
  uint64_t out_msr_data;
  EXPECT_TRUE(msr_.Read(2, &out_msr_data).ok());
  EXPECT_EQ(0x0a0a0a0a0a0a0a0a, out_msr_data);

  uint64_t msr_data = 0xDEADBEEFDEADBEEF;
  EXPECT_TRUE(msr_.Write(1, msr_data).ok());
  EXPECT_TRUE(msr_.Read(1, &out_msr_data).ok());
  EXPECT_EQ(msr_data, out_msr_data);
}

TEST_F(MsrTest, TestSeekFail) {
  uint64_t out_msr_data;
  EXPECT_FALSE(msr_.Read(20, &out_msr_data).ok());
}

TEST_F(MsrTest, TestReadFail) {
  uint64_t out_msr_data;
  EXPECT_FALSE(msr_.Read(5, &out_msr_data).ok());
}

}  // namespace
}  // namespace ecclesia
