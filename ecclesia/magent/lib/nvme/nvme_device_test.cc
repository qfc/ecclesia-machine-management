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

#include "ecclesia/magent/lib/nvme/nvme_device.h"

#include <linux/nvme_ioctl.h>

#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ecclesia/magent/lib/nvme/controller_registers.h"
#include "ecclesia/magent/lib/nvme/identify_namespace.h"
#include "ecclesia/magent/lib/nvme/mock_nvme_device.h"
#include "ecclesia/magent/lib/nvme/nvme_access.h"
#include "ecclesia/magent/lib/nvme/nvme_types.h"

namespace ecclesia {
namespace {

using ::testing::AllOf;
using ::testing::Field;
using ::testing::HasSubstr;
using ::testing::Pointee;
using ::testing::Return;

const uint8_t kNvmeOpcodeSanitizeNvm = 0x84;

class MockNvmeAccessInterface : public NvmeAccessInterface {
 public:
  MOCK_METHOD(absl::Status, ExecuteAdminCommand, (nvme_passthru_cmd * cmd),
              (const, override));
  // MOCK_METHOD(absl::Status, RescanNamespaces, (), (override));
  MOCK_METHOD(absl::Status, ResetSubsystem, (), (override));
  MOCK_METHOD(absl::Status, ResetController, (), (override));
  MOCK_METHOD(absl::StatusOr<ControllerRegisters>, GetControllerRegisters, (),
              (const, override));
};

// Check that operator== works as expected.
TEST(IdentifyNamespaceOperatorEqualsTest, AllFieldsAreConsidered) {
  IdentifyNamespace a{1, 2, 3};
  IdentifyNamespace b{1, 2, 3};
  EXPECT_EQ(a, b);

  IdentifyNamespace c{9, 2, 3};
  EXPECT_NE(a, c);

  IdentifyNamespace d{1, 9, 3};
  EXPECT_NE(a, d);

  IdentifyNamespace e;
  EXPECT_NE(a, e);
}

// Test the typical case: a device with a small number of namespaces.
TEST(EnumerateAllNamespacesTest, ShortList) {
  MockNvmeDevice nvme;

  std::set<uint32_t> expected_set{1, 2, 7};

  EXPECT_CALL(nvme, EnumerateNamespacesAfter(0)).WillOnce(Return(expected_set));

  auto ret = nvme.EnumerateAllNamespaces();
  ASSERT_TRUE(ret.ok());
  EXPECT_THAT(ret.value(), expected_set);
}

// A device which has exactly one commands' worth of NSIDs.
TEST(EnumerateAllNamespacesTest, ExactlyOneCommandsWorthOfNsids) {
  MockNvmeDevice nvme;

  std::set<uint32_t> expected_set;
  for (int i = 0; i < kIdentifyListNamespacesCapacity; ++i) {
    expected_set.insert(i * 7);
  }

  EXPECT_CALL(nvme, EnumerateNamespacesAfter(0)).WillOnce(Return(expected_set));
  EXPECT_CALL(
      nvme, EnumerateNamespacesAfter((kIdentifyListNamespacesCapacity - 1) * 7))
      .WillOnce(Return(std::set<uint32_t>()));

  auto ret = nvme.EnumerateAllNamespaces();
  ASSERT_TRUE(ret.ok());
  EXPECT_THAT(ret.value(), expected_set);
}

// A device which has more than two commands' worth of NSIDs.
TEST(EnumerateAllNamespacesTest, ThreeCommandsWorthOfNsids) {
  MockNvmeDevice nvme;

  std::set<uint32_t> set_1, set_2, set_3;
  for (int i = 0; i < kIdentifyListNamespacesCapacity; ++i) {
    set_1.insert(i + kIdentifyListNamespacesCapacity * 0);
  }
  for (int i = 0; i < kIdentifyListNamespacesCapacity; ++i) {
    set_2.insert(i + kIdentifyListNamespacesCapacity * 1);
  }
  for (int i = 0; i < kIdentifyListNamespacesCapacity / 3; ++i) {
    set_3.insert(i + kIdentifyListNamespacesCapacity * 2);
  }
  std::set<uint32_t> expected_set;
  expected_set.insert(set_1.begin(), set_1.end());
  expected_set.insert(set_2.begin(), set_2.end());
  expected_set.insert(set_3.begin(), set_3.end());

  EXPECT_CALL(nvme, EnumerateNamespacesAfter(0)).WillOnce(Return(set_1));
  EXPECT_CALL(nvme, EnumerateNamespacesAfter(*set_1.rbegin()))
      .WillOnce(Return(set_2));
  EXPECT_CALL(nvme, EnumerateNamespacesAfter(*set_2.rbegin()))
      .WillOnce(Return(set_3));

  auto ret = nvme.EnumerateAllNamespaces();
  ASSERT_TRUE(ret.ok());
  EXPECT_THAT(ret.value(), expected_set);
}

TEST(EnumerateAllNamespacesAndInfoTest, IdentifyNamespaceErrorPropagates) {
  MockNvmeDevice nvme;

  std::set<uint32_t> namespaces{1, 2, 7};

  EXPECT_CALL(nvme, EnumerateNamespacesAfter(0)).WillOnce(Return(namespaces));
  EXPECT_CALL(nvme, GetNamespaceInfo(1))
      .WillOnce(Return(absl::UnavailableError("BOGUS ERROR")));

  EXPECT_EQ(nvme.EnumerateAllNamespacesAndInfo().status().code(),
            absl::StatusCode::kUnavailable);
}

TEST(EnumerateAllNamespacesAndInfoTest, IdentifySeveralNamespacesWithInfo) {
  MockNvmeDevice nvme;

  std::set<uint32_t> namespaces{1, 2, 7};

  EXPECT_CALL(nvme, EnumerateNamespacesAfter(0)).WillOnce(Return(namespaces));

  std::map<uint32_t, IdentifyNamespace> expected = {
      {1, {0x100000, 1 << 9, 111}},
      {2, {0x200000, 1 << 12, 222}},
      {7, {0x300000, 1 << 15, 333}},
  };

  EXPECT_CALL(nvme, GetNamespaceInfo(1)).WillOnce(Return(expected[1]));
  EXPECT_CALL(nvme, GetNamespaceInfo(2)).WillOnce(Return(expected[2]));
  EXPECT_CALL(nvme, GetNamespaceInfo(7)).WillOnce(Return(expected[7]));

  auto ret = nvme.EnumerateAllNamespacesAndInfo();
  ASSERT_TRUE(ret.ok());
  EXPECT_THAT(ret.value(), expected);
}

TEST(IndexOfFormatWithLbaSizeTest, FindMatching) {
  MockNvmeDevice nvme;

  const std::vector<LBAFormat> supported_formats{
      {0, 9, 2},
      {0, 12, 1},
      {0, 16, 0},
  };

  EXPECT_CALL(nvme, GetSupportedLbaFormats())
      .Times(3)
      .WillRepeatedly(Return(supported_formats));

  auto ret = nvme.IndexOfFormatWithLbaSize(1 << 9);
  ASSERT_TRUE(ret.ok());
  EXPECT_EQ(ret.value(), 0);

  ret = nvme.IndexOfFormatWithLbaSize(1 << 12);
  ASSERT_TRUE(ret.ok());
  EXPECT_EQ(ret.value(), 1);

  ret = nvme.IndexOfFormatWithLbaSize(1 << 16);
  ASSERT_TRUE(ret.ok());
  EXPECT_EQ(ret.value(), 2);
}

TEST(IndexOfFormatWithLbaSizeTest, MustHaveMetadataSizeZero) {
  MockNvmeDevice nvme;

  const std::vector<LBAFormat> supported_formats{
      {10, 12, 0},
  };

  EXPECT_CALL(nvme, GetSupportedLbaFormats())
      .WillOnce(Return(supported_formats));

  EXPECT_EQ(nvme.IndexOfFormatWithLbaSize(1 << 12).status().code(),
            absl::StatusCode::kNotFound);
}

TEST(IndexOfFormatWithLbaSizeTest, ReturnsHighestPerformance) {
  MockNvmeDevice nvme;

  const std::vector<LBAFormat> supported_formats{
      {0, 12, 1},
      {0, 12, 0},
      {0, 12, 2},
  };

  EXPECT_CALL(nvme, GetSupportedLbaFormats())
      .WillOnce(Return(supported_formats));

  auto ret = nvme.IndexOfFormatWithLbaSize(1 << 12);
  ASSERT_TRUE(ret.ok());
  EXPECT_EQ(ret.value(), 1);
}

TEST(IndexOfFormatWithLbaSizeTest, NoSupportedFormats) {
  MockNvmeDevice nvme;

  const std::vector<LBAFormat> supported_formats{};

  EXPECT_CALL(nvme, GetSupportedLbaFormats())
      .WillOnce(Return(supported_formats));

  EXPECT_EQ(nvme.IndexOfFormatWithLbaSize(1 << 12).status().code(),
            absl::StatusCode::kNotFound);
}

TEST(SanitizeTest, ExitFailureModeSuccess) {
  auto MockAccess = absl::make_unique<MockNvmeAccessInterface>();
  EXPECT_CALL(
      *MockAccess,
      ExecuteAdminCommand(AllOf(
          Pointee(Field(&nvme_passthru_cmd::opcode, kNvmeOpcodeSanitizeNvm)),
          Pointee(Field(&nvme_passthru_cmd::cdw10, kSanitizeExitFailureMode)))))
      .WillOnce(Return(absl::OkStatus()));
  auto nvme = CreateNvmeDevice(std::move(MockAccess));
  EXPECT_EQ(absl::OkStatus(), nvme->SanitizeExitFailureMode());
}

TEST(SanitizeTest, CryptoSanitizeSuccess) {
  auto MockAccess = absl::make_unique<MockNvmeAccessInterface>();
  EXPECT_CALL(
      *MockAccess,
      ExecuteAdminCommand(AllOf(
          Pointee(Field(&nvme_passthru_cmd::opcode, kNvmeOpcodeSanitizeNvm)),
          Pointee(Field(&nvme_passthru_cmd::cdw10, kSanitizeCryptoErase)))))
      .WillOnce(Return(absl::OkStatus()));
  auto nvme = CreateNvmeDevice(std::move(MockAccess));
  EXPECT_EQ(absl::OkStatus(), nvme->SanitizeCryptoErase());
}

TEST(SanitizeTest, BlockSanitizeSuccess) {
  auto MockAccess = absl::make_unique<MockNvmeAccessInterface>();
  EXPECT_CALL(
      *MockAccess,
      ExecuteAdminCommand(AllOf(
          Pointee(Field(&nvme_passthru_cmd::opcode, kNvmeOpcodeSanitizeNvm)),
          Pointee(Field(&nvme_passthru_cmd::cdw10, kSanitizeBlockErase)))))
      .WillOnce(Return(absl::OkStatus()));
  auto nvme = CreateNvmeDevice(std::move(MockAccess));
  EXPECT_EQ(absl::OkStatus(), nvme->SanitizeBlockErase());
}

TEST(SanitizeTest, OverWriteSanitizeSuccess) {
  auto MockAccess = absl::make_unique<MockNvmeAccessInterface>();
  EXPECT_CALL(
      *MockAccess,
      ExecuteAdminCommand(AllOf(
          Pointee(Field(&nvme_passthru_cmd::opcode, kNvmeOpcodeSanitizeNvm)),
          Pointee(Field(&nvme_passthru_cmd::cdw10,
                        static_cast<uint32_t>(1 << 4 | kSanitizeOverwrite))),
          Pointee(Field(&nvme_passthru_cmd::cdw11, 0x12)))))
      .WillOnce(Return(absl::OkStatus()));
  auto nvme = CreateNvmeDevice(std::move(MockAccess));
  EXPECT_EQ(absl::OkStatus(), nvme->SanitizeOverwrite(0x12));
}

TEST(SanitizeTest, ExitFailureModeFail) {
  auto MockAccess = absl::make_unique<MockNvmeAccessInterface>();
  EXPECT_CALL(
      *MockAccess,
      ExecuteAdminCommand(AllOf(
          Pointee(Field(&nvme_passthru_cmd::opcode, kNvmeOpcodeSanitizeNvm)),
          Pointee(Field(&nvme_passthru_cmd::cdw10, kSanitizeExitFailureMode)))))
      .WillOnce(Return(absl::InternalError("SOME ERRORS")));
  auto nvme = CreateNvmeDevice(std::move(MockAccess));
  auto status = nvme->SanitizeExitFailureMode();
  EXPECT_EQ(status.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(status.ToString(), HasSubstr("SOME ERRORS"));
}

TEST(SanitizeTest, CryptoSanitizeFail) {
  auto MockAccess = absl::make_unique<MockNvmeAccessInterface>();
  EXPECT_CALL(
      *MockAccess,
      ExecuteAdminCommand(AllOf(
          Pointee(Field(&nvme_passthru_cmd::opcode, kNvmeOpcodeSanitizeNvm)),
          Pointee(Field(&nvme_passthru_cmd::cdw10, kSanitizeCryptoErase)))))
      .WillOnce(Return(absl::InternalError("SOME ERRORS")));
  auto nvme = CreateNvmeDevice(std::move(MockAccess));
  auto status = nvme->SanitizeCryptoErase();
  EXPECT_EQ(status.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(status.ToString(), HasSubstr("SOME ERRORS"));
}

TEST(SanitizeTest, BlockSanitizeFail) {
  auto MockAccess = absl::make_unique<MockNvmeAccessInterface>();
  EXPECT_CALL(
      *MockAccess,
      ExecuteAdminCommand(AllOf(
          Pointee(Field(&nvme_passthru_cmd::opcode, kNvmeOpcodeSanitizeNvm)),
          Pointee(Field(&nvme_passthru_cmd::cdw10, kSanitizeBlockErase)))))
      .WillOnce(Return(absl::InternalError("SOME ERRORS")));
  auto nvme = CreateNvmeDevice(std::move(MockAccess));
  auto status = nvme->SanitizeBlockErase();
  EXPECT_EQ(status.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(status.ToString(), HasSubstr("SOME ERRORS"));
}

TEST(SanitizeTest, OverWriteSanitizeFail) {
  auto MockAccess = absl::make_unique<MockNvmeAccessInterface>();
  EXPECT_CALL(
      *MockAccess,
      ExecuteAdminCommand(AllOf(
          Pointee(Field(&nvme_passthru_cmd::opcode, kNvmeOpcodeSanitizeNvm)),
          Pointee(Field(&nvme_passthru_cmd::cdw10,
                        static_cast<uint32_t>(1 << 4 | kSanitizeOverwrite))),
          Pointee(Field(&nvme_passthru_cmd::cdw11, 0x12345678)))))
      .WillOnce(Return(absl::InternalError("SOME ERRORS")));
  auto nvme = CreateNvmeDevice(std::move(MockAccess));
  auto status = nvme->SanitizeOverwrite(0x12345678);
  EXPECT_EQ(status.code(), absl::StatusCode::kInternal);
  EXPECT_THAT(status.ToString(), HasSubstr("SOME ERRORS"));
}

}  // namespace
}  // namespace ecclesia
