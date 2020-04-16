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

#include "lib/redfish/test_mockup.h"

#include <memory>
#include <string>
#include <vector>

#include "base/logging.h"
#include "devtools/build/runtime/get_runfiles_dir.h"
#include "file/base/path.h"
#include "net/util/ports.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "lib/redfish/interface.h"
#include "lib/redfish/raw.h"
#include "util/process/shellprocess.h"

namespace libredfish {

TestingMockupServer::TestingMockupServer(
    absl::string_view mockup_sar_google3_path)
    : port_(::net_util::PickUnusedPortOrDie()) {
  std::string mockup_path =
      file::JoinPath(devtools_build::GetRunfilesDir(), mockup_sar_google3_path);
  // We need to run in IPv6 mode to work in the Blaze testing environment.
  std::vector<std::string> argv = {
      mockup_path, "--host", "::", "--port", absl::StrCat(port_), "--ipv6"};
  mockup_server_ =
      absl::make_unique<ShellProcess>(absl::StrJoin(argv, " "), nullptr);
  CHECK(mockup_server_->Started()) << "Failed to start mockup server.";

  std::string line;
  bool server_ready = false;
  // Note that GetLine can hang indefinitely if the Mockup Server does not
  // output any additional lines (e.g. Mockup never emits the expected line,
  // or Mockup itself is hung). The only indication of this would be the
  // unit test timing out.
  // TODO(dchanman): Add an async wrapper around GetLine so that we can
  // time out the tests faster.
  while (mockup_server_->GetLine(&line)) {
    CHECK_EQ(mockup_server_->exit_status(), -1)
        << "Mockup server exited early.";
    // A hacky check to see when the server is ready is to check for the line
    // printed with "running Server". This couples the tests with the logger
    // output from the redfishTestingMockupServer implementation unfortunately.
    //
    // We dump the stdout from the mockup server to LOG(INFO) in case something
    // unexpected happens and we fail to encounter the "running Server" line
    // to aid debuggability. We don't expect there to be more than ~10 lines of
    // stdout in the working case.
    LOG(INFO) << "[Mockup Server stdout]: " << line;
    if (line == "running Server...") {
      server_ready = true;
      break;
    }
  }
  CHECK(server_ready) << "Mockup server did not come up.";
}

TestingMockupServer::~TestingMockupServer() {
  if (mockup_server_) {
    mockup_server_->Kill(9);
  }
  ::net_util::RecycleUnusedPort(port_);
}

std::unique_ptr<RedfishInterface>
TestingMockupServer::RedfishClientInterface() {
  std::string endpoint = absl::StrCat("[::]:", port_);
  auto intf = libredfish::NewRawInterface(endpoint);
  CHECK(intf.get() != nullptr) << "Could not connect to Redfish Mockup server.";
  return intf;
}

TestingMockupServer::Config TestingMockupServer::GetConfig() const {
  Config config;
  config.hostname = "[::]";
  config.port = port_;
  return config;
}

}  // namespace libredfish
