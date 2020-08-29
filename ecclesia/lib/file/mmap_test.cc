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

#include "ecclesia/lib/file/mmap.h"

#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/lib/file/test_filesystem.h"

namespace ecclesia {
namespace {

using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::IsEmpty;

class MappedMemoryTest : public ::testing::Test {
 protected:
  MappedMemoryTest() : fs_(GetTestTempdirPath()) {}

  TestFilesystem fs_;
};

TEST_F(MappedMemoryTest, ReadOnlyWorksOnSimpleFile) {
  fs_.CreateFile("/a.txt", "0123456789\n");
  auto maybe_mmap =
      MappedMemory::OpenReadOnly(fs_.GetTruePath("/a.txt"), 0, 11);
  ASSERT_TRUE(maybe_mmap.has_value());

  MappedMemory mmap = std::move(*maybe_mmap);
  EXPECT_EQ(mmap.MemoryAsStringView(), "0123456789\n");

  // The span should be the same as the string view.
  EXPECT_THAT(mmap.MemoryAsStringView(),
              ElementsAreArray(mmap.MemoryAsReadOnlySpan()));

  // The writable span should be empty.
  EXPECT_THAT(mmap.MemoryAsReadWriteSpan(), IsEmpty());
}

TEST_F(MappedMemoryTest, ReadOnlyFailsOnMissingFile) {
  EXPECT_THAT(MappedMemory::OpenReadOnly(fs_.GetTruePath("/b.txt"), 0, 11),
              Eq(absl::nullopt));
}

TEST_F(MappedMemoryTest, ReadOnlyWorksOnSmallerFile) {
  fs_.CreateFile("/c.txt", "0123456789\n");
  auto maybe_mmap =
      MappedMemory::OpenReadOnly(fs_.GetTruePath("/c.txt"), 0, 64);
  ASSERT_TRUE(maybe_mmap.has_value());

  MappedMemory mmap = std::move(*maybe_mmap);
  EXPECT_EQ(mmap.MemoryAsStringView().size(), 64);
  EXPECT_EQ(mmap.MemoryAsStringView().substr(0, 11), "0123456789\n");
  EXPECT_EQ(mmap.MemoryAsStringView().substr(11, 64 - 11),
            std::string(64 - 11, '\0'));
}

TEST_F(MappedMemoryTest, ReadOnlyWorksWithOffset) {
  fs_.CreateFile("/d.txt", "0123456789\n");
  auto maybe_mmap = MappedMemory::OpenReadOnly(fs_.GetTruePath("/d.txt"), 5, 6);
  ASSERT_TRUE(maybe_mmap.has_value());

  MappedMemory mmap = std::move(*maybe_mmap);
  EXPECT_EQ(mmap.MemoryAsStringView(), "56789\n");
}

}  // namespace
}  // namespace ecclesia
