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

#include "mmaster/middles/devpath/redfish.h"

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "lib/devpath/devpath.h"
#include "lib/redfish/result.h"
#include "lib/time/clock.h"
#include "mmaster/backends/redfish/redfish.h"
#include "mmaster/middles/devpath/graph_devpath.h"

namespace ecclesia {

RedfishBackendDevpathUpdater::RedfishBackendDevpathUpdater(
    RedfishBackend *backend, Clock *clock)
    : backend_(backend), clock_(clock) {}

bool RedfishBackendDevpathUpdater::operator()(
    GraphDevpathMapper::PluginSnapshot *snapshot) {
  for (const auto &plugins : backend_->GetPlugins()) {
    if (IsValidDevpath(plugins.devpath) &&
        GetDevpathPlugin(plugins.devpath) == plugins.devpath) {
      snapshot->plugins.push_back(plugins.devpath);
    }
  }
  snapshot->timestamp = clock_->Now();
  return true;
}

}  // namespace ecclesia
