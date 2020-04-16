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

#include "mmaster/middles/devpath/graph_devpath.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "absl/container/flat_hash_map.h"
#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "third_party/boost/allowed/graph/adjacency_list.hpp"
#include "lib/devpath/devpath.h"
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/devpath/graph_generator.h"
#include "mmaster/middles/devpath/graph_inverter.h"
#include "mmaster/middles/devpath/graph_merger.h"

namespace ecclesia {

GraphDevpathMapper::GraphDevpathMapper(
    const MachineMasterConfiguration &mm_config,
    const absl::flat_hash_map<std::string, SnapshotUpdater> &agent_map)
    : merge_spec_(mm_config.merge_spec()) {
  // Populate the agent vector using data from the given map. All snapshots
  // start out with no data and no current timestamp.
  agents_.reserve(agent_map.size());

  for (const AgentConfig &agent : mm_config.agent()) {
    auto fallback_plugins = std::vector<std::string>(
        agent.fallback_plugin().begin(), agent.fallback_plugin().end());

    agents_.push_back(Agent{agent.os_domain(),
                            {{}, absl::InfinitePast()},
                            agent_map.at(agent.os_domain()),
                            std::move(fallback_plugins)});
  }
  // Do an initial population of the agent data and graph snapshot.
  UpdateGraphSnapshot();
}

absl::optional<std::string> GraphDevpathMapper::DomainDevpathToMachineDevpath(
    absl::string_view domain, absl::string_view devpath) const {
  std::shared_ptr<GraphSnapshot> snapshot = std::atomic_load(&graph_snapshot_);
  const DevpathGraph &root = *snapshot->root;

  if (!IsValidDevpath(devpath)) return absl::nullopt;
  DevpathComponents components = GetDevpathComponents(devpath);

  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(root); vi != vi_end; ++vi) {
    for (const auto &loc_devpath : (root[*vi].local_agent_devpaths)) {
      if ((loc_devpath.local_devpath == components.path) &&
          (loc_devpath.local_agent == domain)) {
        components.path = root[*vi].global_devpath;
        return MakeDevpathFromComponents(components);
      }
    }
  }
  return absl::nullopt;
}
absl::optional<std::string> GraphDevpathMapper::MachineDevpathToDomainDevpath(
    absl::string_view domain, absl::string_view devpath) const {
  std::shared_ptr<GraphSnapshot> snapshot = std::atomic_load(&graph_snapshot_);
  const DevpathGraph &root = *snapshot->root;

  if (!IsValidDevpath(devpath)) return absl::nullopt;
  DevpathComponents components = GetDevpathComponents(devpath);

  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(root); vi != vi_end; ++vi) {
    for (const auto &loc_devpath : (root[*vi].local_agent_devpaths)) {
      if ((loc_devpath.local_agent == domain) &&
          root[*vi].global_devpath == components.path) {
        components.path = loc_devpath.local_devpath;
        return MakeDevpathFromComponents(components);
      }
    }
  }
  return absl::nullopt;
}
std::vector<absl::string_view> GraphDevpathMapper::MachineDevpathToDomains(
    absl::string_view devpath) const {
  std::shared_ptr<GraphSnapshot> snapshot = std::atomic_load(&graph_snapshot_);
  const DevpathGraph &root = *snapshot->root;

  std::vector<absl::string_view> agents;

  if (!IsValidDevpath(devpath)) return agents;
  DevpathComponents components = GetDevpathComponents(devpath);

  DevpathVertexIt vi, vi_end;
  for (std::tie(vi, vi_end) = boost::vertices(root); vi != vi_end; ++vi) {
    if (root[*vi].global_devpath == components.path) {
      for (const auto &loc_devpath : root[*vi].local_agent_devpaths) {
        agents.push_back(loc_devpath.local_agent);
      }
    }
  }
  return agents;
}

void GraphDevpathMapper::UpdateGraphSnapshot() {
  // Update all of the agent snapshots. If the base snapshot hasn't yet been
  // initialized then an update is needed; otherwise, only update if one of the
  // agents has an updated snapshot.
  bool graph_update_needed = (std::atomic_load(&graph_snapshot_) == nullptr);
  for (Agent &agent : agents_) {
    if (agent.updater(&agent.snapshot)) {
      graph_update_needed = true;
    }
  }
  // If necessary construct a new graph snapshot and then after it is ready
  // overwrite the current shared pointer.
  if (graph_update_needed) {
    auto new_snapshot = std::make_shared<GraphSnapshot>();
    new_snapshot->root = nullptr;
    absl::flat_hash_map<std::string, DevpathGraph *> agent_graph_map;
    for (const Agent &agent : agents_) {
      // Create and populate a new graph.
      auto graph = absl::make_unique<DevpathGraph>();
      // If the updater returned no plugins, default to the fallback list
      absl::Span<const std::string> plugins = agent.snapshot.plugins.empty()
                                                  ? agent.fallback_plugins
                                                  : agent.snapshot.plugins;
      GraphGenerator generator(plugins);
      CHECK(generator.GenerateGraph(graph.get(), agent.domain));
      agent_graph_map[agent.domain] = graph.get();
      if (agent.domain == merge_spec_.root()) new_snapshot->root = graph.get();
      // Move it into the graphs vector to keep it alive.
      new_snapshot->graphs.push_back(std::move(graph));
    }
    CHECK(InvertPluginGraphs(agent_graph_map, merge_spec_));
    CHECK(MergePluginGraphs(agent_graph_map, merge_spec_));
    std::atomic_store(&graph_snapshot_, std::move(new_snapshot));
  }
}

}  // namespace ecclesia
