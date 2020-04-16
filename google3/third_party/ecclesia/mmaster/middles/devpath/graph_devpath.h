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

// Defines a class which implements the DevpathMapper interface
#ifndef ECCLESIA_MMASTER_MIDDLES_DEVPATH_GRAPH_DEVPATH_H_
#define ECCLESIA_MMASTER_MIDDLES_DEVPATH_GRAPH_DEVPATH_H_

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/devpath/devpath.h"
#include "mmaster/middles/devpath/graph_generator.h"

namespace ecclesia {

class GraphDevpathMapper : public DevpathMapper {
 public:
  // Captures a snapshot of all the plugins from a specific domain.
  struct PluginSnapshot {
    std::vector<std::string> plugins;
    absl::Time timestamp;
  };
  // Standard update function that will take a snapshot and (possibly) update
  // the data, returning true if the data was updated and false otherwise.
  //
  // Implementers of this function do not need to make it thread-safe. The
  // updater will only be called during a mapper update, and updates are not
  // executed concurrently.
  using SnapshotUpdater = std::function<bool(PluginSnapshot *)>;
  using SnapshotUpdaterMap = absl::flat_hash_map<std::string, SnapshotUpdater>;

  // Construct a mapper using the given merge spec and an updater for every
  // agent used as a data source. The map should be a map of agent name ->
  // updater for that agent.
  GraphDevpathMapper(const MachineMasterConfiguration &mm_config,
                     const SnapshotUpdaterMap &agent_map);

  GraphDevpathMapper(const GraphDevpathMapper &other) = delete;
  GraphDevpathMapper &operator=(const GraphDevpathMapper &other) = delete;

  absl::optional<std::string> DomainDevpathToMachineDevpath(
      absl::string_view domain, absl::string_view devpath) const override;
  absl::optional<std::string> MachineDevpathToDomainDevpath(
      absl::string_view domain, absl::string_view devpath) const override;
  std::vector<absl::string_view> MachineDevpathToDomains(
      absl::string_view devpath) const override;

 private:
  // Update all of the agent snapshots, and if any of those snapshots have
  // changed update the graph snapshot as well.
  //
  // The graph snapshot will be updated by constructing a new snapshot and then
  // overwriting the shared pointer using atomic_store, so as long as all users
  // of the data operate on a copy they grabbed via atomic_load, they are
  // thread-safe.
  //
  // This function may modify data contained in agents_ without any other
  // synchronization, so it is NOT safe to call it multiple times without any
  // external synchronization (e.g. by only calling it from one thread).
  // Additionally, the updater functions are not required to be thread-safe
  // and so even if access to agents_ was synchronized it would still not be
  // safe to call this concurrently without also fixing all updaters.
  void UpdateGraphSnapshot();

  // The merge spec used to combine individual agent data.
  const PluginGraphMergeSpec merge_spec_;

  // Captures all the necessary information from the agents supplying data to
  // the mapper: the name of the agent, the most recent snapshot of plugin data
  // for it and the updater function for updating a snapshot.
  //
  // This data is not protected by any synchronization, and should only be used
  // by UpdateGraphSnapshot to rebuild graph_snapshot_.
  struct Agent {
    std::string domain;
    PluginSnapshot snapshot;
    SnapshotUpdater updater;
    std::vector<std::string> fallback_plugins;
  };
  std::vector<Agent> agents_;

  // The most recent snapshot of devpath graph data along with a pointer to the
  // entry in the vector that represents the root graph.
  //
  // For thread safety, all users of the snapshot should only interact with it
  // via atomic_load and atomic_store.
  struct GraphSnapshot {
    const DevpathGraph *root;
    std::vector<std::unique_ptr<const DevpathGraph>> graphs;
  };
  std::shared_ptr<GraphSnapshot> graph_snapshot_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MIDDLES_DEVPATH_GRAPH_DEVPATH_H_
