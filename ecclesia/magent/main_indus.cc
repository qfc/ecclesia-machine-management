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

// Redfish server for the Ecclesia Management Agent on Indus
#include <stdlib.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/memory/memory.h"
#include "lib/smbios/platform_translator.h"
#include "magent/main_common.h"
#include "magent/redfish/indus/redfish_service.h"
#include "magent/sysmodel/fru.h"
#include "magent/sysmodel/sysmodel.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"

ABSL_FLAG(int, port, 3995, "Port number for the magent to listen on");
ABSL_FLAG(std::string, assemblies_dir, "/etc/google/magent",
          "Path to a directory containing JSON Assemblies");
ABSL_FLAG(std::string, mced_socket_path, "/var/run/mced2.socket",
          "Path to the mced unix domain socket");

namespace {
using tensorflow::serving::net_http::EventExecutor;
using tensorflow::serving::net_http::HTTPServerInterface;
using tensorflow::serving::net_http::ServerOptions;

constexpr char kSmbiosEntryPointPath[] =
    "/sys/firmware/dmi/tables/smbios_entry_point";
constexpr char kSmbiosTablesPath[] = "/sys/firmware/dmi/tables/DMI";
constexpr char kSysfsMemFilePath[] = "/dev/mem";
}  // namespace

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  if (absl::GetFlag(FLAGS_assemblies_dir).empty()) {
    std::cerr << "a non-empty --assemblies_dir must be specified" << std::endl;
    return EXIT_FAILURE;
  }

  ecclesia::SysmodelParams params = {
      .field_translator =
          absl::make_unique<ecclesia::IndusSmbiosFieldTranslator>(),
      .smbios_entry_point_path = kSmbiosEntryPointPath,
      .smbios_tables_path = kSmbiosTablesPath,
      .mced_socket_path = absl::GetFlag(FLAGS_mced_socket_path),
      .sysfs_mem_file_path = kSysfsMemFilePath,
      .frus = {ecclesia::FruInstance{
          .fru_name = "motherboard",
          // TODO(dwangsf): Update this hardcoded FRU information with SMBUS
          // addrs for accessing the FRU.
          .info = ecclesia::FruInfo{
              .manufacturer = "Quanta",
              .serial_number = "MBBQTW194201573",
              .part_number = "1043652-02",
          }}}};

  std::unique_ptr<ecclesia::SystemModel> system_model =
      absl::make_unique<ecclesia::SystemModel>(std::move(params));

  auto server = ecclesia::CreateServer(absl::GetFlag(FLAGS_port));
  ecclesia::IndusRedfishService redfish_service(
      server.get(), system_model.get(), absl::GetFlag(FLAGS_assemblies_dir));

  bool success = server->StartAcceptingRequests();
  if (server != nullptr && success) {
    server->WaitForTermination();
    return EXIT_SUCCESS;
  } else {
    std::cerr << "Failed to start the server." << std::endl;
    return EXIT_FAILURE;
  }
}
