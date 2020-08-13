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

#include "ecclesia/lib/redfish/test_mockup.h"

#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <string>

#include "absl/base/macros.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "absl/types/variant.h"
#include "ecclesia/lib/file/path.h"
#include "ecclesia/lib/file/test_filesystem.h"
#include "ecclesia/lib/logging/globals.h"
#include "ecclesia/lib/logging/logging.h"
#include "ecclesia/lib/logging/posix.h"
#include "ecclesia/lib/network/testing.h"
#include "ecclesia/lib/redfish/interface.h"
#include "ecclesia/lib/redfish/raw.h"

namespace libredfish {
namespace {

// Constants defining how long to wait and sleep while waiting for the daemon
// to start and make its serving available.
constexpr absl::Duration kDaemonStartTimeout = absl::Seconds(30);
constexpr absl::Duration kDaemonStartSleepDuration = absl::Milliseconds(50);

std::string ConfigToEndpoint(TestingMockupServer::ConfigUnix config) {
  return absl::StrCat("unix://", config.socket_path);
}

std::string ConfigToEndpoint(TestingMockupServer::ConfigNetwork config) {
  return absl::StrCat(config.hostname, ":", config.port);
}

}  // namespace

TestingMockupServer::TestingMockupServer(absl::string_view mockup_shar,
                                         absl::string_view uds_path) {
  config_ = ConfigUnix{.socket_path = uds_path.data()};
  std::string mockup_path = ecclesia::GetTestDataDependencyPath(
      ecclesia::JoinFilePaths("redfish_mockups", mockup_shar));
  std::string string_argv[] = {mockup_path, "--unix", uds_path.data()};
  char *argv[ABSL_ARRAYSIZE(string_argv) + 1] = {};
  for (size_t i = 0; i < ABSL_ARRAYSIZE(string_argv); ++i) {
    argv[i] = &string_argv[i][0];
  }
  SetUpMockupServer(argv);
}

TestingMockupServer::TestingMockupServer(absl::string_view mockup_shar) {
  config_ = ConfigNetwork{.hostname = "[::1]",
                          .port = ecclesia::FindUnusedPortOrDie()};
  std::string mockup_path = ecclesia::GetTestDataDependencyPath(
      ecclesia::JoinFilePaths("redfish_mockups", mockup_shar));
  std::string string_argv[] = {
      mockup_path,
      "--host",
      "::1",
      "--port",
      absl::StrCat(std::get<ConfigNetwork>(config_).port),
      "--ipv6"};
  char *argv[ABSL_ARRAYSIZE(string_argv) + 1] = {};
  for (size_t i = 0; i < ABSL_ARRAYSIZE(string_argv); ++i) {
    argv[i] = &string_argv[i][0];
  }
  SetUpMockupServer(argv);
}

void TestingMockupServer::SetUpMockupServer(char **server_argv) {
  // Launch the supprocess using spawn.
  absl::Time start_time = absl::Now();
  absl::Time give_up_time = start_time + kDaemonStartTimeout;
  int result = posix_spawn(&server_pid_, server_argv[0], nullptr, nullptr,
                           server_argv, nullptr);
  ecclesia::Check(result == 0, "mockup server process started")
      << "posix_spawn() returned " << result;

  // Wait for the client to be up.
  auto client = RedfishClientInterface();
  bool server_ready = false;
  do {
    // Try to fetch the redfish URI from the server. If it works, we're running.
    if (client->GetUri("/redfish").AsObject()) {
      server_ready = true;
    } else {
      // If fetching the URI failed, check to make sure the process is still
      // running. If it isn't then terminate with a fatal log.
      int status;
      pid_t waited = waitpid(server_pid_, &status, WNOHANG);
      if (waited == -1) {
        ecclesia::PosixFatalLog() << "waitpid() failed";
      } else if (waited == 0) {
        // This is the good case, it means we're still waiting, so do nothing.
      } else {
        // The process terminated in some way, try and log a useful indicator of
        // how it terminated.
        if (WIFEXITED(status)) {
          ecclesia::FatalLog()
              << "mockup server terminated early with exit code "
              << WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
          ecclesia::FatalLog() << "mockup server terminated early with signal "
                               << WTERMSIG(status);
        } else {
          ecclesia::FatalLog() << "mockup server terminated in an unknown way";
        }
      }
      // Wait a little while before trying again.
      absl::SleepFor(kDaemonStartSleepDuration);
    }
  } while (!server_ready && absl::Now() < give_up_time);
  ecclesia::Check(server_ready, "mockup server came up");
}

TestingMockupServer::~TestingMockupServer() {
  kill(server_pid_, SIGKILL);
  waitpid(server_pid_, nullptr, 0);
}

std::unique_ptr<RedfishInterface>
TestingMockupServer::RedfishClientInterface() {
  std::string endpoint =
      absl::visit([](auto &conn) { return ConfigToEndpoint(conn); }, config_);
  auto intf = libredfish::NewRawInterface(endpoint);
  ecclesia::Check(intf != nullptr, "can connect to the redfish mockup server");
  return intf;
}

absl::variant<TestingMockupServer::ConfigNetwork,
              TestingMockupServer::ConfigUnix>
TestingMockupServer::GetConfig() const {
  return config_;
}

}  // namespace libredfish
