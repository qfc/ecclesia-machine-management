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

#include "lib/redfish/sysmodel.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "lib/redfish/property.h"
#include "lib/redfish/result.h"
#include "lib/redfish/testing/json_mockup.h"
#include "lib/redfish/topology.h"
#include "lib/time/clock_fake.h"

namespace libredfish {
namespace {

using testing::ElementsAre;

DEFINE_REDFISH_PROPERTY(IntProperty, int, "IntProperty",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(StringProperty, std::string, "StringProperty",
                        absl::InfiniteDuration());

DEFINE_REDFISH_PROPERTY(VolatileInt, int, "VolatileInt", absl::Seconds(5));
DEFINE_REDFISH_PROPERTY(UncacheableInt, int, "UncacheableInt",
                        absl::ZeroDuration());

// Helper for calling sysmodel->QueryAll and returning all results in an array.
template <typename PropertyDefinitionT>
std::vector<libredfish::Result<typename PropertyDefinitionT::type>>
QueryAllToArray(libredfish::Sysmodel *sysmodel) {
  std::vector<libredfish::Result<typename PropertyDefinitionT::type>> results;
  sysmodel->QueryAll<PropertyDefinitionT>(
      [&results](auto r) { results.push_back(std::move(r)); });
  return std::move(results);
}

// Helper for calling sysmodel->Query and returning all results in an array.
template <typename PropertyDefinitionT>
std::vector<libredfish::Result<typename PropertyDefinitionT::type>>
QueryToArray(libredfish::Sysmodel *sysmodel,
             absl::Span<const absl::string_view> devpaths) {
  std::vector<libredfish::Result<typename PropertyDefinitionT::type>> results;
  sysmodel->Query<PropertyDefinitionT>(
      devpaths, [&results](auto r) { results.push_back(std::move(r)); });
  return std::move(results);
}

TEST(Sysmodel, ReturnsComponentsAsDevpathsAndPlugins) {
  auto component1 = absl::make_unique<Component>();
  component1->name = "plugin1_name";
  component1->local_devpath = "/phys/plugin1";
  component1->type = Component::Type::kBoard;

  auto component2 = absl::make_unique<Component>();
  component2->name = "dev";
  component2->local_devpath = "/phys/plugin1:device:dev";
  component2->type = Component::Type::kConnector;

  auto component3 = absl::make_unique<Component>();
  component3->name = "conn";
  component3->local_devpath = "/phys/plugin1:connector:conn";
  component3->type = Component::Type::kDevice;

  auto component4 = absl::make_unique<Component>();
  component4->name = "plugin2_name";
  component4->local_devpath = "/phys/plugin2";
  component4->type = Component::Type::kBoard;

  Assembly assembly;
  assembly.components.push_back(std::move(component1));
  assembly.components.push_back(std::move(component2));
  assembly.components.push_back(std::move(component3));
  assembly.components.push_back(std::move(component4));

  Topology topology;
  topology.assemblies.push_back(std::move(assembly));

  auto redfish_intf = NewJsonMockupInterface("{}");
  PropertyRegistry property_registry;
  ecclesia::FakeClock clock;

  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));

  EXPECT_THAT(sysmodel.GetDevpaths(),
              ElementsAre("/phys/plugin1", "/phys/plugin1:device:dev",
                          "/phys/plugin1:connector:conn", "/phys/plugin2"));

  EXPECT_THAT(
      sysmodel.GetPlugins(),
      ElementsAre(Result<std::string>{"/phys/plugin1", "plugin1_name"},
                  Result<std::string>{"/phys/plugin2", "plugin2_name"}));
}

TEST(Sysmodel, NoAssemblyQueriesEmpty) {
  Topology topology;
  auto redfish_intf = NewJsonMockupInterface("{}");
  PropertyRegistry property_registry;
  ecclesia::FakeClock clock;

  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));
  auto results = QueryAllToArray<IntProperty>(&sysmodel);
  EXPECT_TRUE(results.empty());
}

TEST(Sysmodel, ConstantQueryWorks) {
  absl::Time now = absl::Now();

  auto component1 = absl::make_unique<Component>();
  component1->local_devpath = "/phys/thing1";
  component1->properties.Set<IntProperty>(42, "uri_source", now);
  component1->properties.Set<StringProperty>("p1", "uri_source", now);

  auto component2 = absl::make_unique<Component>();
  component2->local_devpath = "/phys/thing2";
  component2->properties.Set<IntProperty>(43, "uri_source", now);

  Assembly assembly;
  assembly.components.push_back(std::move(component1));
  assembly.components.push_back(std::move(component2));

  Topology topology;
  topology.assemblies.push_back(std::move(assembly));

  auto redfish_intf = NewJsonMockupInterface("{}");
  PropertyRegistry property_registry;
  ecclesia::FakeClock clock;

  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));

  // Query for something that no components have
  sysmodel.QueryAll<UncacheableInt>(
      [&](auto r) { FAIL() << "Unexpected element: " << r; });

  // Query for something which only 1 component has
  EXPECT_THAT(QueryAllToArray<StringProperty>(&sysmodel),
              ElementsAre(Result<std::string>{"/phys/thing1", "p1"}));
  // Query for something which all components have
  EXPECT_THAT(QueryAllToArray<IntProperty>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing1", 42},
                          Result<int>{"/phys/thing2", 43}));
}

TEST(Sysmodel, DevpathQueryWorks) {
  absl::Time now = absl::Now();
  Topology topology;

  auto component1 = absl::make_unique<Component>();
  component1->local_devpath = "/phys/thing1";
  component1->properties.Set<IntProperty>(42, "uri_source", now);
  component1->properties.Set<StringProperty>("p1", "uri_source", now);
  topology.devpath_to_component_map["/phys/thing1"] = component1.get();

  auto component2 = absl::make_unique<Component>();
  component2->local_devpath = "/phys/thing2";
  component2->properties.Set<IntProperty>(43, "uri_source", now);
  topology.devpath_to_component_map["/phys/thing2"] = component2.get();

  Assembly assembly;
  assembly.components.push_back(std::move(component1));
  assembly.components.push_back(std::move(component2));

  topology.assemblies.push_back(std::move(assembly));

  auto redfish_intf = NewJsonMockupInterface("{}");
  PropertyRegistry property_registry;
  ecclesia::FakeClock clock;

  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));

  // Query for a single property on a devpath
  EXPECT_THAT(QueryToArray<StringProperty>(&sysmodel, {"/phys/thing1"}),
              ElementsAre(Result<std::string>{"/phys/thing1", "p1"}));
  EXPECT_THAT(QueryToArray<IntProperty>(&sysmodel, {"/phys/thing2"}),
              ElementsAre(Result<int>{"/phys/thing2", 43}));
  // Query for missing property on a devpath
  EXPECT_THAT(QueryToArray<StringProperty>(&sysmodel, {"/phys/thing2"}),
              ElementsAre());

  // Query for something that no components have
  EXPECT_THAT(
      QueryToArray<UncacheableInt>(&sysmodel, {"/phys/thing1", "/phys/thing2"}),
      ElementsAre());
  // Query for something which only 1 component has
  EXPECT_THAT(
      QueryToArray<StringProperty>(&sysmodel, {"/phys/thing1", "/phys/thing2"}),
      ElementsAre(Result<std::string>{"/phys/thing1", "p1"}));
  EXPECT_THAT(
      QueryToArray<StringProperty>(&sysmodel, {"/phys/thing2", "/phys/thing1"}),
      ElementsAre(Result<std::string>{"/phys/thing1", "p1"}));
  // Query for something which all components have
  EXPECT_THAT(
      QueryToArray<IntProperty>(&sysmodel, {"/phys/thing1", "/phys/thing2"}),
      ElementsAre(Result<int>{"/phys/thing1", 42},
                  Result<int>{"/phys/thing2", 43}));
  EXPECT_THAT(
      QueryToArray<IntProperty>(&sysmodel, {"/phys/thing2", "/phys/thing1"}),
      ElementsAre(Result<int>{"/phys/thing2", 43},
                  Result<int>{"/phys/thing1", 42}));
}

TEST(Sysmodel, StaleDataIsRefreshed) {
  ecclesia::FakeClock clock;
  absl::Time now = clock.Now();

  Topology topology;

  auto component = absl::make_unique<Component>();
  component->local_devpath = "/phys/thing";
  component->properties.Set<VolatileInt>(7, "Sensor", now);
  topology.uri_to_associated_component_map["Sensor"].push_back(component.get());

  Assembly assembly;
  assembly.components.push_back(std::move(component));
  topology.assemblies.push_back(std::move(assembly));

  auto redfish_intf = NewJsonMockupInterface(R"json(
    {
      "Sensor": {
        "@odata.id": "Sensor",
        "VolatileInt": 8
      }
    }
  )json");
  PropertyRegistry property_registry;
  property_registry.Register<VolatileInt>();

  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));

  // Before the value goes stale, we should read the cached value
  EXPECT_THAT(QueryAllToArray<VolatileInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 7}));
  clock.AdvanceTime(absl::Seconds(3));
  EXPECT_THAT(QueryAllToArray<VolatileInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 7}));

  // After the value goes stale, we should read the new value
  clock.AdvanceTime(absl::Seconds(3));
  EXPECT_THAT(QueryAllToArray<VolatileInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 8}));
}

TEST(Sysmodel, UncacheableDataCanBeFetched) {
  ecclesia::FakeClock clock;
  absl::Time now = clock.Now();

  Topology topology;

  auto component = absl::make_unique<Component>();
  component->local_devpath = "/phys/thing";
  component->properties.Set<UncacheableInt>(7, "Sensor", now);
  topology.uri_to_associated_component_map["Sensor"].push_back(component.get());

  Assembly assembly;
  assembly.components.push_back(std::move(component));
  topology.assemblies.push_back(std::move(assembly));

  auto redfish_intf = NewJsonMockupInterface(R"json(
    {
      "Sensor": {
        "@odata.id": "Sensor",
        "UncacheableInt": 8
      }
    }
  )json");
  PropertyRegistry property_registry;
  property_registry.Register<UncacheableInt>();

  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));

  // Make sure the edge condition where the collection timestamp and expiry
  // timestamp are equal doesn't cause strange behaviour. In this case, we still
  // consider the value as valid
  EXPECT_THAT(QueryAllToArray<UncacheableInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 7}));
  // Any change in time should immediately invalidate this value
  clock.AdvanceTime(absl::Nanoseconds(1));
  EXPECT_THAT(QueryAllToArray<UncacheableInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 8}));
}

TEST(Sysmodel, MissingDataOnRefreshIsNotReturned) {
  ecclesia::FakeClock clock;
  absl::Time now = clock.Now();

  Topology topology;

  auto component = absl::make_unique<Component>();
  component->local_devpath = "/phys/thing";
  component->properties.Set<VolatileInt>(7, "Sensor", now);
  topology.uri_to_associated_component_map["Sensor"].push_back(component.get());

  Assembly assembly;
  assembly.components.push_back(std::move(component));
  topology.assemblies.push_back(std::move(assembly));

  auto redfish_intf = NewJsonMockupInterface(R"json(
    {
      "Sensor": {
        "@odata.id": "Sensor"
      }
    }
  )json");
  PropertyRegistry property_registry;
  property_registry.Register<VolatileInt>();
  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));

  // Before the value goes stale, we should read the cached value
  EXPECT_THAT(QueryAllToArray<VolatileInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 7}));

  // After the value goes stale, the new value is missing so nothing is returned
  clock.AdvanceTime(absl::Seconds(6));
  EXPECT_THAT(QueryAllToArray<VolatileInt>(&sysmodel), ElementsAre());
}

TEST(Sysmodel, FetchingStaleDataUpdatesOtherDataToo) {
  ecclesia::FakeClock clock;
  absl::Time now = clock.Now();

  Topology topology;

  auto component = absl::make_unique<Component>();
  component->local_devpath = "/phys/thing";
  component->properties.Set<IntProperty>(5, "Sensor", now);
  component->properties.Set<VolatileInt>(6, "Sensor", now);
  component->properties.Set<UncacheableInt>(7, "Sensor", now);
  topology.uri_to_associated_component_map["Sensor"].push_back(component.get());

  Assembly assembly;
  assembly.components.push_back(std::move(component));
  topology.assemblies.push_back(std::move(assembly));

  auto redfish_intf = NewJsonMockupInterface(R"json(
    {
      "Sensor": {
        "@odata.id": "Sensor",
        "IntProperty": 8,
        "VolatileInt": 9,
        "UncacheableInt": 10
      }
    }
  )json");
  PropertyRegistry property_registry;
  property_registry.Register<IntProperty>();
  property_registry.Register<VolatileInt>();
  property_registry.Register<UncacheableInt>();
  clock.AdvanceTime(absl::Milliseconds(1));

  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));

  // Cacheable values should return the cached value since they're still valid
  EXPECT_THAT(QueryAllToArray<IntProperty>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 5}));
  EXPECT_THAT(QueryAllToArray<VolatileInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 6}));

  // Querying the uncacheable value will trigger a GET on all data
  EXPECT_THAT(QueryAllToArray<UncacheableInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 10}));

  // Previously cached data will now be updated
  EXPECT_THAT(QueryAllToArray<IntProperty>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 8}));
  EXPECT_THAT(QueryAllToArray<VolatileInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing", 9}));
}

TEST(Sysmodel, FetchingStaleDataUpdatesOtherComponentsToo) {
  ecclesia::FakeClock clock;
  absl::Time now = clock.Now();

  Topology topology;

  auto component1 = absl::make_unique<Component>();
  component1->local_devpath = "/phys/thing1";
  component1->properties.Set<IntProperty>(5, "Sensor", now);
  topology.uri_to_associated_component_map["Sensor"].push_back(
      component1.get());

  auto component2 = absl::make_unique<Component>();
  component2->local_devpath = "/phys/thing2";
  component2->properties.Set<UncacheableInt>(7, "Sensor", now);
  topology.uri_to_associated_component_map["Sensor"].push_back(
      component2.get());

  Assembly assembly;
  assembly.components.push_back(std::move(component1));
  assembly.components.push_back(std::move(component2));
  topology.assemblies.push_back(std::move(assembly));

  auto redfish_intf = NewJsonMockupInterface(R"json(
    {
      "Sensor": {
        "@odata.id": "Sensor",
        "IntProperty": 6,
        "UncacheableInt": 8
      }
    }
  )json");
  PropertyRegistry property_registry;
  property_registry.Register<IntProperty>();
  property_registry.Register<UncacheableInt>();
  clock.AdvanceTime(absl::Milliseconds(1));

  Sysmodel sysmodel(redfish_intf.get(), &property_registry, &clock,
                    std::move(topology));

  // Cacheable values should return the cached value since they're still valid
  EXPECT_THAT(QueryAllToArray<IntProperty>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing1", 5}));

  // Querying the stale value will trigger a GET on all data
  // TODO(dchanman): This could be considered strange behaviour. How did we
  // end up in this state where two components with the same source have
  // differing properties? Is it okay for us to have the query return incomplete
  // data in this case? On a subsequent query, the components should be
  // reconciled. Relevant bugs: b/146008362, b/145775513
  EXPECT_THAT(QueryAllToArray<UncacheableInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing2", 8}));
  EXPECT_THAT(QueryAllToArray<UncacheableInt>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing1", 8},
                          Result<int>{"/phys/thing2", 8}));

  // Previously cached data will now be updated. The scraper will extract all
  // data onto both Components.
  EXPECT_THAT(QueryAllToArray<IntProperty>(&sysmodel),
              ElementsAre(Result<int>{"/phys/thing1", 6},
                          Result<int>{"/phys/thing2", 6}));
}

}  // namespace
}  // namespace libredfish
