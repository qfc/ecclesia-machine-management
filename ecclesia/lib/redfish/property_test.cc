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

#include "lib/redfish/property.h"

#include <memory>
#include <optional>
#include <string>

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "lib/redfish/interface.h"
#include "lib/redfish/testing/json_mockup.h"

namespace libredfish {
namespace {

using testing::Eq;
template <typename T>
using CachedValue = PropertyContainer::CachedValue<T>;

DEFINE_REDFISH_PROPERTY(StringProperty, std::string, "StringProperty",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(IntProperty, int, "IntProperty",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(DoubleProperty, double, "DoubleProperty",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(BoolProperty, bool, "BoolProperty",
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(IgnoredProperty, int, "IgnoredProperty",  // NOLINT
                        absl::InfiniteDuration());
DEFINE_REDFISH_PROPERTY(VolatileInt, int, "VolatileInt", absl::Minutes(5));
DEFINE_REDFISH_PROPERTY(UncacheableInt, int, "UncacheableInt",
                        absl::ZeroDuration());

TEST(PropertyContainer, TypedGetWorks) {
  PropertyContainer container;
  auto now = absl::Now();

  container.Set<StringProperty>("stringval", "string_uri", now);
  container.Set<IntProperty>(7, "int_uri", now);
  container.Set<DoubleProperty>(4.2, "double_uri", now);
  container.Set<BoolProperty>(true, "bool_uri", now);

  EXPECT_THAT(container.Get<StringProperty>(),
              Eq(CachedValue<std::string>("stringval", "string_uri",
                                          absl::InfiniteFuture())));
  EXPECT_THAT(container.Get<IntProperty>(),
              Eq(CachedValue<int>(7, "int_uri", absl::InfiniteFuture())));
  EXPECT_THAT(
      container.Get<DoubleProperty>(),
      Eq(CachedValue<double>(4.2, "double_uri", absl::InfiniteFuture())));
  EXPECT_THAT(container.Get<BoolProperty>(),
              Eq(CachedValue<bool>(true, "bool_uri", absl::InfiniteFuture())));
}

TEST(PropertyContainer, ExpirationCorrectlySet) {
  PropertyContainer container;
  auto now = absl::Now();

  container.Set<VolatileInt>(7, "int_uri", now);
  container.Set<UncacheableInt>(8, "int_uri", now);

  EXPECT_THAT(container.Get<VolatileInt>(),
              Eq(CachedValue<int>(7, "int_uri", now + absl::Minutes(5))));
  EXPECT_THAT(container.Get<UncacheableInt>(),
              Eq(CachedValue<int>(8, "int_uri", now)));
}

TEST(PropertyContainer, MissingElementFails) {
  PropertyContainer container;

  auto s1 = container.Get<IntProperty>();
  EXPECT_FALSE(s1.has_value())
      << "Getting a property which has not been set should fail.";
}

TEST(PropertyExtraction, RegisteredExtractFunctionsWork) {
  PropertyRegistry registry;
  auto now = absl::Now();
  registry.Register<IntProperty>();
  registry.Register<DoubleProperty>();
  registry.Register<BoolProperty>();
  registry.Register<StringProperty>();

  auto json_intf = libredfish::NewJsonMockupInterface(R"json(
  {
    "@odata.id": "/redfish/v1/SomeResource",
    "IntProperty": 75,
    "DoubleProperty": 4.2,
    "StringProperty": "Lalala",
    "BoolProperty": true
  }
  )json");
  auto root_obj = json_intf->GetRoot().AsObject();
  ASSERT_TRUE(root_obj);

  PropertyContainer container;

  registry.ExtractAllProperties(root_obj.get(), now, &container);
  EXPECT_THAT(container.Get<IntProperty>(),
              Eq(CachedValue<int>(75, "/redfish/v1/SomeResource",
                                  absl::InfiniteFuture())));
  EXPECT_THAT(container.Get<DoubleProperty>(),
              Eq(CachedValue<double>(4.2, "/redfish/v1/SomeResource",
                                     absl::InfiniteFuture())));
  EXPECT_THAT(container.Get<StringProperty>(),
              Eq(CachedValue<std::string>("Lalala", "/redfish/v1/SomeResource",
                                          absl::InfiniteFuture())));
  EXPECT_THAT(container.Get<BoolProperty>(),
              Eq(CachedValue<bool>(true, "/redfish/v1/SomeResource",
                                   absl::InfiniteFuture())));
}

TEST(PropertyExtraction, MissingPropertiesAreIgnored) {
  PropertyRegistry registry;
  auto now = absl::Now();
  registry.Register<IntProperty>();
  registry.Register<DoubleProperty>();
  registry.Register<BoolProperty>();

  auto json_intf = libredfish::NewJsonMockupInterface(R"json(
  {
    "@odata.id": "/redfish/v1/SomeResource",
    "IntProperty": 75,
    "BoolProperty": false
  }
  )json");
  auto root_obj = json_intf->GetRoot().AsObject();
  ASSERT_TRUE(root_obj);

  PropertyContainer container;

  registry.ExtractAllProperties(root_obj.get(), now, &container);
  EXPECT_THAT(container.Get<IntProperty>(),
              Eq(CachedValue<int>(75, "/redfish/v1/SomeResource",
                                  absl::InfiniteFuture())));
  EXPECT_FALSE(container.Get<StringProperty>().has_value());
  EXPECT_THAT(container.Get<BoolProperty>(),
              Eq(CachedValue<bool>(false, "/redfish/v1/SomeResource",
                                   absl::InfiniteFuture())));
}

TEST(PropertyExtraction, UnregisteredPropertiesAreIgnored) {
  PropertyRegistry registry;
  auto now = absl::Now();

  auto json_intf = libredfish::NewJsonMockupInterface(R"json(
  {
    "IgnoredProperty": 75,
    "IntProperty": 76
  }
  )json");
  auto root_obj = json_intf->GetRoot().AsObject();
  ASSERT_TRUE(root_obj);

  PropertyContainer container;

  registry.ExtractAllProperties(root_obj.get(), now, &container);
  EXPECT_FALSE(container.Get<IgnoredProperty>().has_value());
  EXPECT_FALSE(container.Get<IntProperty>().has_value());
}

TEST(PropertyExtraction, ExpirationCorrectlySet) {
  PropertyRegistry registry;
  auto now = absl::Now();
  registry.Register<IntProperty>();
  registry.Register<VolatileInt>();
  registry.Register<UncacheableInt>();

  auto json_intf = libredfish::NewJsonMockupInterface(R"json(
  {
    "@odata.id": "/redfish/v1/SomeResource",
    "IntProperty": 75,
    "VolatileInt": 76,
    "UncacheableInt": 77
  }
  )json");
  auto root_obj = json_intf->GetRoot().AsObject();
  ASSERT_TRUE(root_obj);

  PropertyContainer container;

  registry.ExtractAllProperties(root_obj.get(), now, &container);
  EXPECT_THAT(container.Get<IntProperty>(),
              Eq(CachedValue<int>(75, "/redfish/v1/SomeResource",
                                  absl::InfiniteFuture())));
  EXPECT_THAT(container.Get<VolatileInt>(),
              Eq(CachedValue<int>(76, "/redfish/v1/SomeResource",
                                  now + absl::Minutes(5))));
  EXPECT_THAT(container.Get<UncacheableInt>(),
              Eq(CachedValue<int>(77, "/redfish/v1/SomeResource", now)));
}

}  // namespace
}  // namespace libredfish
