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

#include <string>
#include <vector>

#include "base/logging.h"
#include "net/proto2/public/text_format.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/container/flat_hash_map.h"
#include "mmaster/config/config.proto.h"

namespace ecclesia {
namespace {

using ::testing::ElementsAre;

class GraphDevpathMapperTest : public ::testing::Test {
 protected:
  GraphDevpathMapperTest() {
    // Define a series of updater functions that will populate a snapshot with
    // a fixed set of devpaths on their first call, and then do no updates on
    // future calls.
    agent_map_["a1"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
      if (!snapshot->plugins.empty()) return false;

      snapshot->plugins = std::vector<std::string>({
          "/phys/A/B",
          "/phys/A",
          "/phys",
      });
      return true;
    };
    agent_map_["a2"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
      if (!snapshot->plugins.empty()) return false;

      snapshot->plugins = std::vector<std::string>({
          "/phys/C/D",
          "/phys/C",
          "/phys",
      });
      return true;
    };
    agent_map_["a3"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
      if (!snapshot->plugins.empty()) return false;

      snapshot->plugins = std::vector<std::string>({
          "/phys/E/F",
          "/phys/E",
          "/phys",
      });
      return true;
    };
    agent_map_["a4"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
      if (!snapshot->plugins.empty()) return false;

      snapshot->plugins = std::vector<std::string>({
          "/phys/G/H",
          "/phys/G",
          "/phys",
      });
      return true;
    };

    // Agent "a1" is the root node.
    // Agent "a2" has a node which plugs into a node in "a1".
    // Agent "a3" has a node which is same as a node in "a1".
    // Agent "a4" has a node which plugs into a node in "a2".
    CHECK(proto2::TextFormat::ParseFromString(
        R"pb(
          agent {
            name: "a1"
            os_domain: "a1"
            fallback_plugin: "/phys"
            fallback_plugin: "/phys/A"
            fallback_plugin: "/phys/A/B"
          }
          agent {
            name: "a2"
            os_domain: "a2"
            fallback_plugin: "/phys"
            fallback_plugin: "/phys/C"
          }
          agent { name: "a3" os_domain: "a3" fallback_plugin: "/phys" }
          agent {
            name: "a4"
            os_domain: "a4"
            fallback_plugin: "/phys"
            fallback_plugin: "/phys/G"
            fallback_plugin: "/phys/G/H"
          }

          merge_spec {
            root: "a1"

            merge_ops {
              base_agent: "a1"
              appendant_agent: "a2"
              plugged_in_node {
                base_devpath: "/phys/A/B"
                appendant_devpath: "/phys"
                connector: "PADS1"
              }
            }

            merge_ops {
              base_agent: "a1"
              appendant_agent: "a3"
              same_node { base_devpath: "/phys/A/B" appendant_devpath: "/phys" }
            }

            merge_ops {
              base_agent: "a2"
              appendant_agent: "a4"
              plugged_in_node {
                base_devpath: "/phys/C"
                appendant_devpath: "/phys/G/H"
                connector: "PADS2"
              }
            }
          }
        )pb",
        &mm_config_));
  }

  GraphDevpathMapper::SnapshotUpdaterMap agent_map_;
  MachineMasterConfiguration mm_config_;
};

TEST_F(GraphDevpathMapperTest, DomainDevpathToMachineDevpath) {
  GraphDevpathMapper mapper(mm_config_, agent_map_);
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C"),
              "/phys/A/B/PADS1/C");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C/D"),
              "/phys/A/B/PADS1/C/D");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E"),
              "/phys/A/B/E");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E/F"),
              "/phys/A/B/E/F");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G"),
              "/phys/A/B/PADS1/C/PADS2/G");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G/H"),
              "/phys/A/B/PADS1/C/PADS2/G/H");
}

TEST_F(GraphDevpathMapperTest, MachineDevpathToDomainDevpath) {
  GraphDevpathMapper mapper(mm_config_, agent_map_);
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a2", "/phys/A/B/PADS1/C"),
              "/phys/C");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a2", "/phys/A/B/PADS1/C/D"),
              "/phys/C/D");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a3", "/phys/A/B/E"),
              "/phys/E");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a3", "/phys/A/B/E/F"),
              "/phys/E/F");
  EXPECT_THAT(
      mapper.MachineDevpathToDomainDevpath("a4", "/phys/A/B/PADS1/C/PADS2/G"),
      "/phys/G");
  EXPECT_THAT(
      mapper.MachineDevpathToDomainDevpath("a4", "/phys/A/B/PADS1/C/PADS2/G/H"),
      "/phys/G/H");
}

TEST_F(GraphDevpathMapperTest, MachineDevpathToDomains) {
  GraphDevpathMapper mapper(mm_config_, agent_map_);
  EXPECT_THAT(mapper.MachineDevpathToDomains("/phys/A"), ElementsAre("a1"));
  EXPECT_THAT(mapper.MachineDevpathToDomains("/phys/A/B"),
              ElementsAre("a1", "a3"));
  EXPECT_THAT(mapper.MachineDevpathToDomains("/phys/A/B/PADS1"),
              ElementsAre("a2"));
  EXPECT_THAT(mapper.MachineDevpathToDomains("/phys/A/B/PADS1/C/D"),
              ElementsAre("a2"));
  EXPECT_THAT(mapper.MachineDevpathToDomains("/phys/A/B/E/F"),
              ElementsAre("a3"));
  EXPECT_THAT(mapper.MachineDevpathToDomains("/phys/A/B/PADS1/C/PADS2/G"),
              ElementsAre("a4"));
  EXPECT_THAT(mapper.MachineDevpathToDomains("/phys/A/B/PADS1/C/PADS2/G/H"),
              ElementsAre("a4"));
}

TEST_F(GraphDevpathMapperTest, MissingAgentA1) {
  // Make agent "a1" report no plugins. We want to make sure that the rest of
  // the agents report consistent devpaths
  agent_map_["a1"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
    snapshot->plugins.clear();
    return true;
  };
  GraphDevpathMapper mapper(mm_config_, agent_map_);

  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C"),
              "/phys/A/B/PADS1/C");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C/D"),
              "/phys/A/B/PADS1/C/D");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E"),
              "/phys/A/B/E");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E/F"),
              "/phys/A/B/E/F");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G"),
              "/phys/A/B/PADS1/C/PADS2/G");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G/H"),
              "/phys/A/B/PADS1/C/PADS2/G/H");

  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C"),
              "/phys/A/B/PADS1/C");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C/D"),
              "/phys/A/B/PADS1/C/D");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E"),
              "/phys/A/B/E");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E/F"),
              "/phys/A/B/E/F");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G"),
              "/phys/A/B/PADS1/C/PADS2/G");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G/H"),
              "/phys/A/B/PADS1/C/PADS2/G/H");
}

TEST_F(GraphDevpathMapperTest, MissingAgentA2) {
  // Make agent "a2" report no plugins. We want to make sure that the rest of
  // the agents report consistent devpaths
  agent_map_["a2"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
    snapshot->plugins.clear();
    return true;
  };
  GraphDevpathMapper mapper(mm_config_, agent_map_);

  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E"),
              "/phys/A/B/E");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E/F"),
              "/phys/A/B/E/F");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G"),
              "/phys/A/B/PADS1/C/PADS2/G");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G/H"),
              "/phys/A/B/PADS1/C/PADS2/G/H");

  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a3", "/phys/A/B/E"),
              "/phys/E");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a3", "/phys/A/B/E/F"),
              "/phys/E/F");
  EXPECT_THAT(
      mapper.MachineDevpathToDomainDevpath("a4", "/phys/A/B/PADS1/C/PADS2/G"),
      "/phys/G");
  EXPECT_THAT(
      mapper.MachineDevpathToDomainDevpath("a4", "/phys/A/B/PADS1/C/PADS2/G/H"),
      "/phys/G/H");
}

TEST_F(GraphDevpathMapperTest, MissingAgentA3) {
  // Make agent "a3" report no plugins. We want to make sure that the rest of
  // the agents report consistent devpaths

  agent_map_["a3"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
    snapshot->plugins.clear();
    return true;
  };
  GraphDevpathMapper mapper(mm_config_, agent_map_);

  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C"),
              "/phys/A/B/PADS1/C");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C/D"),
              "/phys/A/B/PADS1/C/D");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G"),
              "/phys/A/B/PADS1/C/PADS2/G");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G/H"),
              "/phys/A/B/PADS1/C/PADS2/G/H");

  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a2", "/phys/A/B/PADS1/C"),
              "/phys/C");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a2", "/phys/A/B/PADS1/C/D"),
              "/phys/C/D");
  EXPECT_THAT(
      mapper.MachineDevpathToDomainDevpath("a4", "/phys/A/B/PADS1/C/PADS2/G"),
      "/phys/G");
  EXPECT_THAT(
      mapper.MachineDevpathToDomainDevpath("a4", "/phys/A/B/PADS1/C/PADS2/G/H"),
      "/phys/G/H");
}

TEST_F(GraphDevpathMapperTest, MissingAgentA4) {
  // Make agent "a4" report no plugins. We want to make sure that the rest of
  // the agents report consistent devpaths

  agent_map_["a4"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
    snapshot->plugins.clear();
    return true;
  };
  GraphDevpathMapper mapper(mm_config_, agent_map_);

  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C"),
              "/phys/A/B/PADS1/C");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C/D"),
              "/phys/A/B/PADS1/C/D");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E"),
              "/phys/A/B/E");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys/E/F"),
              "/phys/A/B/E/F");

  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a2", "/phys/A/B/PADS1/C"),
              "/phys/C");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a2", "/phys/A/B/PADS1/C/D"),
              "/phys/C/D");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a3", "/phys/A/B/E"),
              "/phys/E");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a3", "/phys/A/B/E/F"),
              "/phys/E/F");
}

TEST_F(GraphDevpathMapperTest, EveryAgentFails) {
  // Make all of the agents fail. We should get still get a consistent set of
  // devpaths out of the mapper using the fallbacks.
  for (auto &entry : agent_map_) {
    entry.second = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
      return false;
    };
  }
  GraphDevpathMapper mapper(mm_config_, agent_map_);

  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a2", "/phys/C"),
              "/phys/A/B/PADS1/C");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a3", "/phys"), "/phys/A/B");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("a4", "/phys/G/H"),
              "/phys/A/B/PADS1/C/PADS2/G/H");

  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A"), "/phys/A");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a1", "/phys/A/B"),
              "/phys/A/B");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a2", "/phys/A/B/PADS1/C"),
              "/phys/C");
  EXPECT_THAT(mapper.MachineDevpathToDomainDevpath("a3", "/phys/A/B"), "/phys");
  EXPECT_THAT(
      mapper.MachineDevpathToDomainDevpath("a4", "/phys/A/B/PADS1/C/PADS2/G/H"),
      "/phys/G/H");
}

class GraphDevpathMapperInvertOpsTest : public ::testing::Test {
 protected:
  GraphDevpathMapperInvertOpsTest() {
    agent_map_["cn"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
      if (!snapshot->plugins.empty()) return false;
      snapshot->plugins = std::vector<std::string>({
          "/phys",
      });
      return true;
    };

    agent_map_["hmb"] = [](GraphDevpathMapper::PluginSnapshot *snapshot) {
      if (!snapshot->plugins.empty()) return false;

      snapshot->plugins = std::vector<std::string>({
          "/phys",
          "/phys/J7",
          "/phys/J7/DOWNLINK",
          "/phys/J2",
          "/phys/J3",
      });
      return true;
    };

    // Agent "indus" is the root node.
    // Agent "hmb" plugs into "indus"
    // indus and hmb both report the hammurabi_io at devpaths "/phys/PE0" and
    // "/phys/J7/DOWNLINK" respectively
    CHECK(proto2::TextFormat::ParseFromString(
        R"pb(
          agent { name: "cn" os_domain: "cn" fallback_plugin: "/phys" }

          agent {
            name: "hmb"
            os_domain: "hmb"
            fallback_plugin: "/phys"
            fallback_plugin: "/phys/J7"
            fallback_plugin: "/phys/J7/DOWNLINK"
          }

          merge_spec {
            root: "cn"

            invert_ops {
              agent: "hmb"
              new_root_devpath: "/phys/J7/DOWNLINK"
              upstream_connectors: "DOWNLINK"
              upstream_connectors: "J14"
            }

            merge_ops {
              base_agent: "cn"
              appendant_agent: "hmb"
              plugged_in_node {
                base_devpath: "/phys"
                appendant_devpath: "/phys/J7/DOWNLINK"
                connector: "PE0"
              }
            }
          }
        )pb",
        &mm_config_));
  }

  GraphDevpathMapper::SnapshotUpdaterMap agent_map_;
  MachineMasterConfiguration mm_config_;
};

TEST_F(GraphDevpathMapperInvertOpsTest, NoMissingAgent) {
  GraphDevpathMapper mapper(mm_config_, agent_map_);

  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("cn", "/phys"), "/phys");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("hmb", "/phys"),
              "/phys/PE0/J14/DOWNLINK");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("hmb", "/phys/J7/DOWNLINK"),
              "/phys/PE0");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("hmb", "/phys/J2"),
              "/phys/PE0/J14/DOWNLINK/J2");
  EXPECT_THAT(mapper.DomainDevpathToMachineDevpath("hmb", "/phys/J3"),
              "/phys/PE0/J14/DOWNLINK/J3");
}

}  // namespace
}  // namespace ecclesia
