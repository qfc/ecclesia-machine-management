#include "lib/io/msr.h"

#include <stdint.h>

#include "platforms/util/testing/fake_filesystem.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/googletest.h"
#include "testing/base/public/gunit.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"

namespace ecclesia {
namespace {

class MsrTest : public ::testing::Test {
 protected:
  MsrTest()
      : fs_(FLAGS_test_tmpdir),
        msr_(absl::StrCat(FLAGS_test_tmpdir, "/dev/cpu/0/msr")) {
    fs_.CreateDir("/dev/cpu/0");

    // content: 0a 0a 0a 0a 0a 0a 0a 0a 0a 0a
    fs_.CreateAndWriteFile("/dev/cpu/0/msr", "\n\n\n\n\n\n\n\n\n\n", 10);
  }

  ::platforms_util_testing::FakeFilesystem fs_;
  Msr msr_;
};

TEST(Msr, TestMsrNotExist) {
  uint64_t out_msr_data;
  Msr msr_non_exist(absl::StrCat(FLAGS_test_tmpdir, "/dev/cpu/400/msr"));
  EXPECT_FALSE(msr_non_exist.Read(2, &out_msr_data).ok());

  EXPECT_FALSE(msr_non_exist.Write(1, 0xDEADBEEFDEADBEEF).ok());
}

TEST_F(MsrTest, TestReadWriteOk) {
  uint64_t out_msr_data;
  EXPECT_OK(msr_.Read(2, &out_msr_data));
  EXPECT_EQ(0x0a0a0a0a0a0a0a0a, out_msr_data);

  uint64_t msr_data = 0xDEADBEEFDEADBEEF;
  EXPECT_OK(msr_.Write(1, msr_data));
  EXPECT_OK(msr_.Read(1, &out_msr_data));
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
