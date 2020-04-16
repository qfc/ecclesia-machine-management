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

#ifndef ECCLESIA_LIB_REDFISH_TEST_MOCKUP_H_
#define ECCLESIA_LIB_REDFISH_TEST_MOCKUP_H_

#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "lib/redfish/interface.h"
#include "util/process/shellprocess.h"

namespace libredfish {

// TestingMockupServer spins up a Redfish Mockup Server on localhost and allows
// raw Redfish interfaces to be connected to it.
class TestingMockupServer {
 public:
  // In the BUILD file of your test implementation, ensure that you have
  // included the google3 SAR binary path as a data dependency.
  //
  // For example:
  // cc_test(
  // ...
  //  data = ["//redfish_mockups:indus_hmb_cn_mockup.sar"],
  // ...
  // )
  //
  // Then, provide the mockup_sar_google3_path. For example:
  // mockup_sar_path =
  //  "google3/redfish_mockups/indus_hmb_cn_mockup.sar"
  explicit TestingMockupServer(absl::string_view mockup_sar_google3_path);
  ~TestingMockupServer();

  // Returns a new RedfishInterface connected to the mockup server.
  std::unique_ptr<RedfishInterface> RedfishClientInterface();

  // Data structure containing hostname and port for connecting to the mockup.
  struct Config {
    std::string hostname;
    int port;
  };
  Config GetConfig() const;

 private:
  int port_;
  std::unique_ptr<ShellProcess> mockup_server_;
};

}  // namespace libredfish

#endif  // ECCLESIA_LIB_REDFISH_TEST_MOCKUP_H_
