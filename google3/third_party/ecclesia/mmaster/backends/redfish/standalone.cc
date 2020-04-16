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

// Proof of concept binary which connects to a Redfish server and dumps out
// all of the information that it sees. Not for production use, intended only
// to debug the Redfish backend's development.

#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "lib/redfish/property_definitions.h"
#include "lib/redfish/result.h"
#include "mmaster/backends/redfish/redfish.h"
#include "mmaster/config/config.proto.h"

ABSL_FLAG(std::string, hostname, "localhost",
          "Hostname of the Redfish server.");
ABSL_FLAG(int, port, 8000, "Port number of the Redfish server.");
ABSL_FLAG(std::string, property, "", "Property name to query.");
ABSL_FLAG(bool, plugins, false, "Print plugin names.");

namespace ecclesia {
namespace {

void PrintDevpaths(RedfishBackend *backend) {
  auto devpaths = backend->GetDevpaths();
  std::cout << "Found " << devpaths.size() << " devpaths: \n";
  for (const auto &dp : devpaths) {
    std::cout << dp << "\n";
  }
}

void PrintPlugins(RedfishBackend *backend) {
  auto plugins = backend->GetPlugins();
  std::cout << "Querying: plugins\n";
  for (const auto &result : plugins) {
    std::cout << result.devpath << " : " << result.value << "\n";
  }
}

template <typename PType>
void PrintProperty(RedfishBackend *backend) {
  std::cout << "Querying: " << PType::Name << "\n";
  backend->QueryAll<PType>([](auto result) {
    std::cout << result.devpath << " : " << result.value << "\n";
  });
}

// Map of property name to a function to query and print its results
absl::flat_hash_map<std::string, std::function<void(RedfishBackend *)>>
    &GetGlobalPropertyPrintingRegistry() {
  static auto &registry =
      *(new absl::flat_hash_map<std::string,
                                std::function<void(RedfishBackend *)>>());
  return registry;
}

template <typename PType>
void RegisterProperty() {
  GetGlobalPropertyPrintingRegistry()[PType::Name] = PrintProperty<PType>;
}

int RealMain(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  // Any new properties to be supported must be added here.
  RegisterProperty<libredfish::PropertyCapacityMiB>();
  RegisterProperty<libredfish::PropertyLogicalSizeMiB>();
  RegisterProperty<libredfish::PropertyManufacturer>();
  RegisterProperty<libredfish::PropertyMemoryDeviceType>();
  RegisterProperty<libredfish::PropertyOperatingSpeedMhz>();
  RegisterProperty<libredfish::PropertyPartNumber>();
  RegisterProperty<libredfish::PropertySerialNumber>();

  AgentConfig config;
  config.set_name("redfish");
  config.mutable_redfish()->set_hostname(absl::GetFlag(FLAGS_hostname));
  config.mutable_redfish()->set_port(absl::GetFlag(FLAGS_port));
  RedfishBackend backend(config);

  bool printed = false;
  std::string property = absl::GetFlag(FLAGS_property);
  if (!property.empty()) {
    auto printer_func = GetGlobalPropertyPrintingRegistry().find(property);
    if (printer_func == GetGlobalPropertyPrintingRegistry().end()) {
      std::cerr << "Could not query for property '" << property
                << "', it has not been registered. Registered properties:\n";
      for (const auto &kv : GetGlobalPropertyPrintingRegistry()) {
        std::cerr << kv.first << "\n";
      }
      return 1;
    }
    // Call the registered print function
    printer_func->second(&backend);
    printed = true;
  }

  if (absl::GetFlag(FLAGS_plugins)) {
    PrintPlugins(&backend);
    printed = true;
  }

  // If no print args were provided, just print the devpaths
  if (!printed) {
    PrintDevpaths(&backend);
  }

  return 0;
}

}  // namespace
}  // namespace ecclesia

int main(int argc, char *argv[]) { return ecclesia::RealMain(argc, argv); }
