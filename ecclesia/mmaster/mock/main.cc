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

// This implements a mockable MachineMaster daemon it will run as a standalone
// process, serving the MachineMaster interface using gRPC over a local unix
// domain socket. The server will respond to Enumerate* and Query* requests
// using the contents of canned Query*Response protos it is provided.

#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include "mmaster/mock/service.h"
#include "mmaster/service/service.grpc.pb.h"
#include "net/grpc/public/include/grpc/grpc_security_constants.h"
#include "net/grpc/public/include/grpcpp/security/server_credentials.h"
#include "net/grpc/public/include/grpcpp/server_builder.h"
#include "net/grpc/public/include/grpcpp/server_impl.h"

ABSL_FLAG(std::string, uds_path, "",
          "Path to a local unix domain socket where the service will run");
ABSL_FLAG(std::string, mocks_dir, "",
          "Path to a directory containing mock Query RPC response messages");

namespace ecclesia {
namespace {

int RealMain(int argc, char *argv[]) {
  // Parse command line arguments and fail if required ones were not specified.
  absl::ParseCommandLine(argc, argv);
  if (absl::GetFlag(FLAGS_uds_path).empty()) {
    std::cerr << "a non-empty --uds_path must be specified" << std::endl;
    return 1;
  }
  if (absl::GetFlag(FLAGS_mocks_dir).empty()) {
    std::cerr << "a non-empty --mocks_dir must be specified" << std::endl;
    return 1;
  }

  // Construct the mock mmaster.
  std::unique_ptr<MachineMasterService::Service> machine_master =
      MakeMockService(absl::GetFlag(FLAGS_mocks_dir));

  // Constrct and launch the gRPC server.
  ::grpc::ServerBuilder builder;
  auto creds = ::grpc::experimental::LocalServerCredentials(UDS);
  builder.AddListeningPort(
      absl::StrFormat("unix:%s", absl::GetFlag(FLAGS_uds_path)), creds);
  builder.RegisterService(machine_master.get());
  auto server = builder.BuildAndStart();

  // Wait until the server is shut down and then exit.
  server->Wait();
  return 0;
}

}  // namespace
}  // namespace ecclesia

int main(int argc, char *argv[]) { return ecclesia::RealMain(argc, argv); }
