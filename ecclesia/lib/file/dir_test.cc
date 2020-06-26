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

#include "ecclesia/lib/file/dir.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "ecclesia/lib/file/test_filesystem.h"

namespace ecclesia {
namespace {

namespace fs = std::filesystem;
using ::testing::UnorderedElementsAre;

// Sets up a temporary directory in the test directory in order to ensure no
// filesystem state is passed on from test to test.
class DirTest : public ::testing::Test {
 public:
  DirTest() : testdir_(GetTestTempdirPath("testdir")) {
    fs::create_directory(fs::path(testdir_));
  }

  std::vector<std::string> WithEachFileInDirectoryVector(
      absl::string_view dirname) {
    std::vector<std::string> files;
    WithEachFileInDirectory(dirname, [&files](absl::string_view file) {
      files.push_back(std::string(file));
    });
    return files;
  }

  ~DirTest() { fs::remove_all(fs::path(testdir_)); }

 protected:
  absl::string_view TestDirName() { return testdir_; }

 private:
  std::string testdir_;
};

TEST_F(DirTest, EmptyDirectory) {
  EXPECT_THAT(WithEachFileInDirectoryVector(TestDirName()),
              UnorderedElementsAre());
}

TEST_F(DirTest, DirDoesntExist) {
  std::string bad_dir = absl::StrCat(TestDirName(), "/baddir");
  EXPECT_THAT(WithEachFileInDirectoryVector(bad_dir), UnorderedElementsAre());
}

TEST_F(DirTest, DirIsAFile) {
  fs::path filepath = fs::path(TestDirName()) / "file1";
  std::ofstream touch(filepath);
  EXPECT_TRUE(fs::exists(filepath));

  EXPECT_THAT(WithEachFileInDirectoryVector(filepath.c_str()),
              UnorderedElementsAre());
}

TEST_F(DirTest, OneFile) {
  fs::path filepath = fs::path(TestDirName()) / "file1";
  std::ofstream touch(filepath);
  ASSERT_TRUE(fs::exists(filepath));

  EXPECT_THAT(WithEachFileInDirectoryVector(TestDirName()),
              UnorderedElementsAre("file1"));
}

TEST_F(DirTest, TwoFiles) {
  fs::path f1 = fs::path(TestDirName()) / "file1";
  std::ofstream touch_f1(f1);
  EXPECT_TRUE(fs::exists(f1));

  fs::path f2 = fs::path(TestDirName()) / "file2";
  std::ofstream touch_f2(f2);
  EXPECT_TRUE(fs::exists(f2));

  EXPECT_THAT(WithEachFileInDirectoryVector(TestDirName()),
              UnorderedElementsAre("file1", "file2"));
}

TEST_F(DirTest, SubdirectoriesListed) {
  fs::path subdir = fs::path(TestDirName()) / "subdir";
  fs::create_directories(subdir);
  EXPECT_TRUE(fs::exists(subdir));

  EXPECT_THAT(WithEachFileInDirectoryVector(TestDirName()),
              UnorderedElementsAre("subdir"));
}

}  // namespace
}  // namespace ecclesia
