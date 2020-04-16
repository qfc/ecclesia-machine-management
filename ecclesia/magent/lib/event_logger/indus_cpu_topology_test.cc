#include "magent/lib/event_logger/indus_cpu_topology.h"

#include "platforms/util/testing/fake_filesystem.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/googletest.h"
#include "testing/base/public/gunit.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"

namespace ecclesia {

namespace {

namespace fs = ::std::filesystem;

using testing::ElementsAre;

class IndusCpuTopologyTest : public ::testing::Test {
 protected:
  IndusCpuTopologyTest()
      : fs_(FLAGS_test_tmpdir),
        apifs_(fs::path(
            absl::StrCat(FLAGS_test_tmpdir, "/sys/devices/system/cpu"))) {
    // Create directories.
    fs_.CreateDir("/sys/devices/system/cpu/cpu0/topology");
    fs_.CreateDir("/sys/devices/system/cpu/cpu1/topology");
    fs_.CreateDir("/sys/devices/system/cpu/cpu2/topology");
    fs_.CreateDir("/sys/devices/system/cpu/cpu3/topology");
    fs_.CreateDir("/sys/devices/system/cpu/cpu4/topology");
    fs_.CreateDir("/sys/devices/system/cpu/cpu5/topology");
    fs_.CreateDir("/sys/devices/system/cpu/cpu6/topology");
    fs_.CreateDir("/sys/devices/system/cpu/cpu7/topology");

    // Create sysfs entries for CPU topology.
    fs_.CreateAndWriteFile(
        "/sys/devices/system/cpu/cpu0/topology/physical_package_id", "0", 1);
    fs_.CreateAndWriteFile(
        "/sys/devices/system/cpu/cpu1/topology/physical_package_id", "0", 1);
    fs_.CreateAndWriteFile(
        "/sys/devices/system/cpu/cpu2/topology/physical_package_id", "0", 1);
    fs_.CreateAndWriteFile(
        "/sys/devices/system/cpu/cpu3/topology/physical_package_id", "0", 1);
    fs_.CreateAndWriteFile(
        "/sys/devices/system/cpu/cpu4/topology/physical_package_id", "1", 1);
    fs_.CreateAndWriteFile(
        "/sys/devices/system/cpu/cpu5/topology/physical_package_id", "1", 1);
    fs_.CreateAndWriteFile(
        "/sys/devices/system/cpu/cpu6/topology/physical_package_id", "1", 1);
    fs_.CreateAndWriteFile(
        "/sys/devices/system/cpu/cpu7/topology/physical_package_id", "1", 1);
  }

  ::platforms_util_testing::FakeFilesystem fs_;
  ApifsDirectory apifs_;
};

TEST(IndusCpuTopologyConstructionTest, Ctor) {
  IndusCpuTopology top;
  std::vector<int> lpus = top.GetLpusForSocketId(0);
  EXPECT_FALSE(lpus.empty());
}

TEST_F(IndusCpuTopologyTest, TestParse) {
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/online", "aaa", 3);
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/int", "1234", 4);
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/double", "1.2345", 6);
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/float", "1.23", 4);
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/bool", "true", 4);
  IndusCpuTopology indus_top(
      {.apifs_path = FLAGS_test_tmpdir + "/sys/devices/system/cpu"});

  int int_val;
  EXPECT_FALSE(indus_top.ReadApifsFile<int>("file_none_exist", &int_val).ok());

  EXPECT_FALSE(indus_top.ReadApifsFile<int>("online", &int_val).ok());
  EXPECT_TRUE(indus_top.ReadApifsFile<int>("int", &int_val).ok());
  EXPECT_EQ(1234, int_val);

  float float_val;
  ASSERT_TRUE(indus_top.ReadApifsFile<float>("float", &float_val).ok());
  EXPECT_FLOAT_EQ(1.23, float_val);

  double double_val;
  ASSERT_TRUE(indus_top.ReadApifsFile<double>("double", &double_val).ok());
  EXPECT_DOUBLE_EQ(1.2345, double_val);

  bool bool_val;
  EXPECT_TRUE(indus_top.ReadApifsFile<bool>("bool", &bool_val).ok());
}

TEST_F(IndusCpuTopologyTest, TestCPURangeOnlineFail) {
  IndusCpuTopology indus_top(
      {.apifs_path = FLAGS_test_tmpdir + "/sys/devices/system/cpu"});

  EXPECT_EQ(-1, indus_top.GetSocketIdForLpu(0));
}

TEST_F(IndusCpuTopologyTest, TestCPURangeFromFail) {
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/online", "a-7", 3);
  IndusCpuTopology indus_top(
      {.apifs_path = FLAGS_test_tmpdir + "/sys/devices/system/cpu"});

  EXPECT_EQ(-1, indus_top.GetSocketIdForLpu(0));
}

TEST_F(IndusCpuTopologyTest, TestCPURangeToFail) {
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/online", "0-a", 3);
  IndusCpuTopology indus_top(
      {.apifs_path = FLAGS_test_tmpdir + "/sys/devices/system/cpu"});

  EXPECT_EQ(-1, indus_top.GetSocketIdForLpu(0));
}

TEST_F(IndusCpuTopologyTest, TestGetSocketIdForLpu) {
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/online", "0-7", 3);

  IndusCpuTopology indus_top(
      {.apifs_path = FLAGS_test_tmpdir + "/sys/devices/system/cpu"});

  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(i / 4, indus_top.GetSocketIdForLpu(i));
  }
}

TEST_F(IndusCpuTopologyTest, TestGetLpusForSocketId) {
  fs_.CreateAndWriteFile("/sys/devices/system/cpu/online", "0-7", 3);

  IndusCpuTopology indus_top(
      {.apifs_path = FLAGS_test_tmpdir + "/sys/devices/system/cpu"});

  std::vector<int> lpus = indus_top.GetLpusForSocketId(0);
  EXPECT_THAT(lpus, ElementsAre(0, 1, 2, 3));
  lpus = indus_top.GetLpusForSocketId(1);
  EXPECT_THAT(lpus, ElementsAre(4, 5, 6, 7));
}

}  // namespace

}  // namespace ecclesia
