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

#include "lib/apifs/apifs.h"

#include <sys/stat.h>

#include <string>
#include <vector>

#include "platforms/util/testing/fake_filesystem.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/googletest.h"
#include "testing/base/public/gunit.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"

namespace ecclesia {
namespace {

using ::testing::Not;
using ::testing::UnorderedElementsAre;
using ::testing::status::CanonicalStatusIs;
using ::testing::status::IsOk;

class ApifsTest : public ::testing::Test {
 protected:
  ApifsTest()
      : fs_(FLAGS_test_tmpdir),
        apifs_(absl::StrCat(FLAGS_test_tmpdir, "/sys")) {
    // Create a couple of directories.
    fs_.CreateDir("/sys/ab/cd/ef");
    fs_.CreateDir("/sys/ab/gh");

    // Drop some files into one of the directories.
    fs_.CreateAndWriteFile("/sys/ab/file1", "contents**\n", 11);
    fs_.CreateAndWriteFile("/sys/ab/file2", "x\ny\nz\n", 6);
    fs_.CreateAndWriteFile("/sys/ab/file3", "", 0);

    // Create a file with lots of contents, to verify reading from a file that
    // requires multiple read() calls.
    std::string large_contents(10000, 'J');
    fs_.CreateAndWriteFile("/sys/ab/largefile", large_contents.data(),
                           large_contents.size());

    // Create a symlink to one of the files.
    fs_.CreateSymlink("file1", "/sys/ab/file4");
  }

  ::platforms_util_testing::FakeFilesystem fs_;
  ApifsDirectory apifs_;
};

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
  ApifsFile msr_;
};

TEST_F(ApifsTest, TestExists) {
  EXPECT_TRUE(apifs_.Exists());
  EXPECT_FALSE(apifs_.Exists("a"));
  EXPECT_TRUE(apifs_.Exists("ab"));
  EXPECT_TRUE(apifs_.Exists("ab/cd"));
  EXPECT_TRUE(apifs_.Exists("ab/cd/ef"));
  EXPECT_FALSE(apifs_.Exists("ab/cd/ef/gh"));
  EXPECT_TRUE(apifs_.Exists("ab/gh"));
  EXPECT_TRUE(apifs_.Exists("ab/file1"));
  EXPECT_TRUE(apifs_.Exists("ab/file2"));
  EXPECT_TRUE(apifs_.Exists("ab/file3"));
  EXPECT_TRUE(apifs_.Exists("ab/file4"));
  EXPECT_FALSE(apifs_.Exists("ab/file5"));

  ApifsFile f4(apifs_, "ab/file4");
  EXPECT_TRUE(f4.Exists());
  ApifsFile f5(apifs_, "ab/file5");
  EXPECT_FALSE(f5.Exists());
}

TEST_F(ApifsTest, TestStat) {
  struct stat st;

  EXPECT_OK(apifs_.Stat("", &st));
  EXPECT_TRUE(S_ISDIR(st.st_mode));

  EXPECT_THAT(apifs_.Stat("a", &st), Not(IsOk()));

  EXPECT_OK(apifs_.Stat("ab", &st));
  EXPECT_TRUE(S_ISDIR(st.st_mode));

  EXPECT_OK(apifs_.Stat("ab/file1", &st));
  EXPECT_TRUE(S_ISREG(st.st_mode));

  ApifsFile f1(apifs_, "ab/file1");
  EXPECT_OK(f1.Stat(&st));
  EXPECT_TRUE(S_ISREG(st.st_mode));
}

TEST_F(ApifsTest, TestListEntries) {
  std::vector<std::string> entries;
  // Helper function that will reduce the values in entries from paths to
  // base names. This simplifies the expectation checking.
  auto reduce_entries_to_basenames = [&entries]() {
    for (std::string &entry : entries) {
      auto found = entry.find_last_of('/');
      if (found != entry.npos) {
        entry = entry.substr(found + 1);
      }
    }
  };

  // Look at the root contents.
  EXPECT_OK(apifs_.ListEntries(&entries));
  reduce_entries_to_basenames();
  EXPECT_THAT(entries, UnorderedElementsAre("ab"));

  // Look at the contents of "ab".
  EXPECT_OK(apifs_.ListEntries("ab", &entries));
  reduce_entries_to_basenames();
  EXPECT_THAT(entries, UnorderedElementsAre("cd", "file1", "file2", "file3",
                                            "file4", "gh", "largefile"));

  // Look at the contents of "ab/cd".
  EXPECT_OK(apifs_.ListEntries("ab/cd", &entries));
  reduce_entries_to_basenames();
  EXPECT_THAT(entries, UnorderedElementsAre("ef"));

  // Look at the contents of "ab/cd/ef".
  EXPECT_OK(apifs_.ListEntries("ab/cd/ef", &entries));
  reduce_entries_to_basenames();
  EXPECT_THAT(entries, UnorderedElementsAre());

  // Look at a non-existant "ab/cd/ef/gh".
  EXPECT_THAT(apifs_.ListEntries("ab/cd/ef/gh", &entries),
              CanonicalStatusIs(absl::StatusCode::kNotFound));
}

TEST_F(ApifsTest, TestRead) {
  std::string contents;

  // Look at the contents of each file in ab.
  EXPECT_OK(apifs_.Read("ab/file1", &contents));
  EXPECT_EQ("contents**\n", contents);
  EXPECT_OK(apifs_.Read("ab/file2", &contents));
  EXPECT_EQ("x\ny\nz\n", contents);
  EXPECT_OK(apifs_.Read("ab/file3", &contents));
  EXPECT_EQ("", contents);
  EXPECT_OK(apifs_.Read("ab/file4", &contents));
  EXPECT_EQ("contents**\n", contents);
  EXPECT_OK(apifs_.Read("ab/largefile", &contents));
  EXPECT_EQ(std::string(10000, 'J'), contents);

  // We should not be able to read directories or non-existant files.
  EXPECT_THAT(apifs_.Read("ab", &contents), Not(IsOk()));
  EXPECT_THAT(apifs_.Read("ab/file5", &contents), Not(IsOk()));

  // Use an ApifsFile to do the read.
  ApifsFile f2(apifs_, "ab/file2");
  EXPECT_OK(f2.Read(&contents));
  EXPECT_EQ("x\ny\nz\n", contents);
}

TEST_F(ApifsTest, TestWrite) {
  // Verify that we can write a file and read it back. Depends on Read also
  // working correctly.
  std::string contents;
  EXPECT_OK(apifs_.Read("ab/file3", &contents));
  EXPECT_EQ("", contents);
  EXPECT_OK(apifs_.Write("ab/file3", "hello, world!\n"));
  EXPECT_OK(apifs_.Read("ab/file3", &contents));
  EXPECT_EQ("hello, world!\n", contents);

  // We should not be able to write to directories or non-existant files.
  EXPECT_THAT(apifs_.Write("ab", "testing"), Not(IsOk()));
  EXPECT_THAT(apifs_.Write("ab/file5", "testing"), Not(IsOk()));

  // Use an ApifsFile to do the same operations.
  ApifsFile f3(apifs_, "ab/file3");
  EXPECT_OK(f3.Read(&contents));
  EXPECT_EQ("hello, world!\n", contents);
  EXPECT_OK(f3.Write("goodbye, file!\n"));
  EXPECT_OK(f3.Read(&contents));
  EXPECT_EQ("goodbye, file!\n", contents);
}

TEST_F(ApifsTest, TestReadLink) {
  std::string link;

  // Test reading the symlink from file4 -> file1.
  EXPECT_OK(apifs_.ReadLink("ab/file4", &link));
  EXPECT_EQ("file1", link);

  // Reading a non-symlink should fail, or a non-existant file.
  EXPECT_THAT(apifs_.ReadLink("ab/file1", &link), Not(IsOk()));
  EXPECT_THAT(apifs_.ReadLink("ab/file5", &link), Not(IsOk()));

  // Use an ApifsFile to try the read.
  link.clear();
  ApifsFile f4(apifs_, "ab/file4");
  EXPECT_OK(f4.ReadLink(&link));
  EXPECT_EQ("file1", link);
}

TEST_F(MsrTest, TestSeekAndRead) {
  std::vector<char> out_msr_data(8);
  std::vector<char> expected(8, 0xa);

  EXPECT_OK(msr_.SeekAndRead(2, absl::MakeSpan(out_msr_data)));
  EXPECT_EQ(expected, out_msr_data);
}

TEST_F(MsrTest, TestSeekAndWrite) {
  std::vector<char> out_msr_data(8);
  std::vector<char> expected(8, 0xa);

  EXPECT_OK(msr_.SeekAndRead(2, absl::MakeSpan(out_msr_data)));
  EXPECT_EQ(expected, out_msr_data);

  std::vector<char> expected_msr_data{
    0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  EXPECT_OK(msr_.SeekAndWrite(1, absl::MakeConstSpan(expected_msr_data)));
  EXPECT_OK(msr_.SeekAndRead(1, absl::MakeSpan(out_msr_data)));
  EXPECT_EQ(expected_msr_data, out_msr_data);
}

TEST(MsrTestConstruct, TestMsrNotExist) {
  ApifsFile msr_default;
  std::vector<char> out_msr_data(8);
  EXPECT_FALSE(msr_default.SeekAndRead(2, absl::MakeSpan(out_msr_data)).ok());

  ApifsFile msr_non_exist(absl::StrCat(FLAGS_test_tmpdir, "/dev/cpu/400/msr"));
  EXPECT_FALSE(msr_non_exist.SeekAndRead(2, absl::MakeSpan(out_msr_data)).ok());

  std::vector<char> msr_data{
    0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  EXPECT_FALSE(msr_non_exist.SeekAndWrite(
      1, absl::MakeConstSpan(msr_data)).ok());
}

TEST_F(MsrTest, TestSeekFail) {
  std::vector<char> out_msr_data(8);
  EXPECT_FALSE(msr_.SeekAndRead(20, absl::MakeSpan(out_msr_data)).ok());
}

TEST_F(MsrTest, TestSeekAndReadFail) {
  std::vector<char> out_msr_data(8);
  EXPECT_FALSE(msr_.SeekAndRead(5, absl::MakeSpan(out_msr_data)).ok());
}

}  // namespace
}  // namespace ecclesia
