// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "magent/lib/event_reader/mced_reader.h"

#include <sys/socket.h>

#include <cstdio>
#include <optional>
#include <string>
#include <variant>

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/strings/util.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "magent/lib/event_reader/event_reader.h"
#include "util/libcproxy/libcproxy.h"
#include "util/libcproxy/libcwrapper.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::StrictMock;

namespace ecclesia {
namespace {

// This test is mostly borrowed from gsys' mced_reader_test

class McedaemonSocketProxy : public LibcProxy {
 public:
  // Pick some arbitrary values to use for the fd and FILE*.
  McedaemonSocketProxy() : fd_(0xbeef), file_(reinterpret_cast<FILE *>(0x1)) {}

  bool ShouldProxySocket(int domain, int type, int protocol) override {
    return domain == AF_UNIX && type == SOCK_STREAM && protocol == 0;
  }
  bool ShouldProxyFileDescriptor(int fd) override { return fd == fd_; }
  bool ShouldProxyFile(const FILE *file) override { return file == file_; }

  MOCK_METHOD(int, socket, (int domain, int type, int protocol));
  MOCK_METHOD(FILE *, fdopen, (int fildes, const char *mode));
  MOCK_METHOD(char *, fgets, (char *s, int size, FILE *stream));
  MOCK_METHOD(int, fclose, (FILE * stream));
  MOCK_METHOD(int, connect,
              (int sockfd, const struct sockaddr *serv_addr,
               socklen_t addrlen));
  MOCK_METHOD(int, close, (int fd));

  int fd_;
  FILE *file_;
};

ACTION_P(ReadString, val) { return safestrncpy(arg0, val, 1024); }

class McedaemonReaderTest : public ::testing::Test {
 protected:
  McedaemonReaderTest() {}

  static StrictMock<McedaemonSocketProxy> &LibcProxy() {
    ::testing::FLAGS_gmock_catch_leaked_mocks = false;
    static auto &proxy = *(new StrictMock<McedaemonSocketProxy>());
    return proxy;
  }

  static void SetUpTestSuite() { LibcWrapper::SetLibcProxy(&LibcProxy()); }
};

TEST_F(McedaemonReaderTest, SocketFailure) {
  EXPECT_CALL(LibcProxy(), socket(_, _, _)).WillRepeatedly(Return(-1));
  McedaemonReader mced_reader("dummy_socket");
  // Wait for the reader loop to start fetching the mces
  absl::SleepFor(absl::Seconds(5));
  EXPECT_FALSE(mced_reader.ReadEvent());
}

TEST_F(McedaemonReaderTest, ReadFailure) {
  EXPECT_CALL(LibcProxy(), socket(_, _, _))
      .WillRepeatedly(Return(LibcProxy().fd_));
  EXPECT_CALL(LibcProxy(), connect(LibcProxy().fd_, _, _))
      .WillRepeatedly(Return(0));
  EXPECT_CALL(LibcProxy(), fdopen(LibcProxy().fd_, _))
      .WillRepeatedly(Return(LibcProxy().file_));
  EXPECT_CALL(LibcProxy(), fgets(_, _, LibcProxy().file_))
      .WillRepeatedly(ReturnNull());
  EXPECT_CALL(LibcProxy(), fclose(LibcProxy().file_)).WillRepeatedly(Return(0));

  McedaemonReader mced_reader("dummy_socket");
  // Wait for the reader loop to start fetching the mces
  absl::SleepFor(absl::Seconds(5));
  EXPECT_FALSE(mced_reader.ReadEvent());
}

TEST_F(McedaemonReaderTest, ReadSuccess) {
  ::testing::InSequence s;

  EXPECT_CALL(LibcProxy(), socket(_, _, _))
      .WillRepeatedly(Return(LibcProxy().fd_));
  EXPECT_CALL(LibcProxy(), connect(LibcProxy().fd_, _, _)).WillOnce(Return(0));
  EXPECT_CALL(LibcProxy(), fdopen(LibcProxy().fd_, _))
      .WillOnce(Return(LibcProxy().file_));

  std::string mced_string =
      "%B=-9 %c=4 %S=3 %p=0x00000004 %v=2 %A=0x00830f00 %b=12 %s=0x1234567890 "
      "%a=0x876543210 %m=0x1111111122222222 %y=0x000000004a000142 "
      "%i=0x00000018013b1700 %g=0x0000000012059349 %G=0x40248739 "
      "%t=0x0000000000000004 %T=0x0000000004000000 %C=0x0020 "
      "%I=0x00000032413312da\n";

  EXPECT_CALL(LibcProxy(), fgets(_, _, LibcProxy().file_))
      .WillOnce(ReadString(mced_string.c_str()))
      .WillRepeatedly(Return(nullptr));

  EXPECT_CALL(LibcProxy(), fclose(LibcProxy().file_)).WillOnce(Return(0));

  McedaemonReader mced_reader("dummy_socket");
  absl::SleepFor(absl::Seconds(5));
  auto mce_record = mced_reader.ReadEvent();
  ASSERT_TRUE(mce_record);
  MachineCheck expected{.mci_status = 0x1234567890,
                        .mci_address = 0x876543210,
                        .mci_misc = 0x1111111122222222,
                        .mcg_status = 0x12059349,
                        .tsc = 0x4000000,
                        .time = absl::FromUnixMicros(4),
                        .ip = 0x32413312da,
                        .boot = -9,
                        .cpu = 4,
                        .cpuid_eax = 0x00830f00,
                        .init_apic_id = 0x4,
                        .socket = 3,
                        .mcg_cap = 0x40248739,
                        .cs = 0x20,
                        .bank = 12,
                        .vendor = 2};
  MachineCheck mce = std::get<MachineCheck>(mce_record.value().record);
  EXPECT_EQ(mce.mci_status, expected.mci_status);
  EXPECT_EQ(mce.mci_address, expected.mci_address);
  EXPECT_EQ(mce.mci_misc, expected.mci_misc);
  EXPECT_EQ(mce.mcg_status, expected.mcg_status);
  EXPECT_EQ(mce.tsc, expected.tsc);
  EXPECT_EQ(mce.time, expected.time);
  EXPECT_EQ(mce.ip, expected.ip);
  EXPECT_EQ(mce.boot, expected.boot);
  EXPECT_EQ(mce.cpu, expected.cpu);
  EXPECT_EQ(mce.cpuid_eax, expected.cpuid_eax);
  EXPECT_EQ(mce.init_apic_id, expected.init_apic_id);
  EXPECT_EQ(mce.socket, expected.socket);
  EXPECT_EQ(mce.mcg_cap, expected.mcg_cap);
  EXPECT_EQ(mce.cs, expected.cs);
  EXPECT_EQ(mce.bank, expected.bank);
  EXPECT_EQ(mce.vendor, expected.vendor);
}

}  // namespace

}  // namespace ecclesia
