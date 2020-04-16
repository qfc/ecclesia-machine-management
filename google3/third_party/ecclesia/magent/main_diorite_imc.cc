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

// Redfish server for the Ecclesia Management Agent on Diorite IMC
#include <stdlib.h>

#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/memory/memory.h"
#include "magent/main_common.h"
#include "magent/redfish/diorite/redfish_service.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"

ABSL_FLAG(int, port, 3995, "Port number for the magent to listen on");

ABSL_FLAG(std::string, assemblies_dir, "/etc/google/magent",
          "Path to a directory containing JSON Assemblies");

namespace {
using tensorflow::serving::net_http::HTTPServerInterface;
}  // namespace

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  if (absl::GetFlag(FLAGS_assemblies_dir).empty()) {
    std::cerr << "a non-empty --assemblies_dir must be specified" << std::endl;
    return EXIT_FAILURE;
  }

  auto server = ecclesia::CreateServer(absl::GetFlag(FLAGS_port));
  ecclesia::DioriteRedfishService redfish_service(
      server.get(), absl::GetFlag(FLAGS_assemblies_dir));

  bool success = server->StartAcceptingRequests();
  if (server != nullptr && success) {
    server->WaitForTermination();
    return EXIT_SUCCESS;
  } else {
    std::cerr << "Failed to start the server." << std::endl;
    return EXIT_FAILURE;
  }
}
