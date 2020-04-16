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

#ifndef ECCLESIA_MMASTER_MIDDLES_DEVPATH_REDFISH_H_
#define ECCLESIA_MMASTER_MIDDLES_DEVPATH_REDFISH_H_

#include "lib/time/clock.h"
#include "mmaster/backends/redfish/redfish.h"
#include "mmaster/middles/devpath/graph_devpath.h"

namespace ecclesia {

// An implementation of the SnapshotUpdater function required by the graph
// devpath mapper using data fetched from Redfish.
class RedfishBackendDevpathUpdater {
 public:
  RedfishBackendDevpathUpdater(RedfishBackend *backend, Clock *clock);

  // Implements the actual snapshot update.
  bool operator()(GraphDevpathMapper::PluginSnapshot *snapshot);

 private:
  RedfishBackend *backend_;
  Clock *clock_;
};

};  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MIDDLES_DEVPATH_REDFISH_H_
