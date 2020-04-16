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

#include "lib/devpath/transform.h"

#include <algorithm>
#include <optional>
#include <string>

#include "net/proto2/contrib/parse_proto/parse_text_proto.h"
#include "platforms/gsys/proto/gsys.proto.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "lib/devpath/transform_test.proto.h"

namespace ecclesia {
namespace {

using ::ecclesia_transform_test::ComplicatedMessage;
using ::ecclesia_transform_test::NestedMessage;
using ::ecclesia_transform_test::OneofMessage;
using ::ecclesia_transform_test::RepeatedMessage;
using ::ecclesia_transform_test::SimpleMessage;
using ::platforms_gsys::GetEventLogEventsResponse;
using ::testing::EqualsProto;

TEST(TransformDevpathsToDevpathsTest, EmptyTransform) {
  SimpleMessage message = PARSE_TEXT_PROTO(R"pb(
    devpath: "/phys/A"
    not_devpath: "/phys/B"
    also_devpath: "/phys/C/D"
    not_even_a_string: 4
  )pb");
  SimpleMessage expected_result = message;

  // This should never be called.
  auto func = [](absl::string_view devpath) -> absl::optional<std::string> {
    ADD_FAILURE() << "transform function was incorrectly called";
    return absl::nullopt;
  };

  EXPECT_TRUE(TransformProtobufDevpaths(func, "", &message));
  EXPECT_THAT(message, EqualsProto(expected_result));
}

TEST(TransformDevpathsToDevpathsTest, SuccessfulTransform) {
  SimpleMessage message = PARSE_TEXT_PROTO(R"pb(
    devpath: "/phys/A"
    not_devpath: "/phys/B"
    also_devpath: "/phys/C/D"
    not_even_a_string: 4
  )pb");
  SimpleMessage expected_result = PARSE_TEXT_PROTO(R"pb(
    devpath: "/phys/A/ZZ"
    not_devpath: "/phys/B"
    also_devpath: "/phys/C/D/ZZZ"
    not_even_a_string: 4
  )pb");

  // The transform will append /Z* where the number of Z's is equal to the
  // number of '/' in the original string.
  auto func = [](absl::string_view devpath) -> absl::optional<std::string> {
    std::string result = std::string(devpath);
    result.push_back('/');
    result.append(std::count(devpath.begin(), devpath.end(), '/'), 'Z');
    return result;
  };

  EXPECT_TRUE(
      TransformProtobufDevpaths(func, "devpath,also_devpath", &message));
  EXPECT_THAT(message, EqualsProto(expected_result));
}

TEST(TransformDevpathsToDevpathsTest, IdentityTransformDoesNotSet) {
  // In this case also_devpath is not set. Performing an identity transform
  // should not cause it to be set.
  SimpleMessage message = PARSE_TEXT_PROTO(R"pb(
    devpath: "/phys/A/ZZ"
    not_devpath: "/phys/B"
    not_even_a_string: 4
  )pb");
  SimpleMessage expected_result = message;

  // The transform will return the original value. For fields which are not
  // already set, they should remain unset.
  auto func = [](absl::string_view devpath) -> absl::optional<std::string> {
    return std::string(devpath);
  };

  EXPECT_TRUE(
      TransformProtobufDevpaths(func, "devpath,also_devpath", &message));
  EXPECT_THAT(message, EqualsProto(expected_result));
}

TEST(TransformDevpathsToDevpathsTest, SuccessfulNestedTransform) {
  NestedMessage message = PARSE_TEXT_PROTO(R"pb(
    sub { devpath: "/phys" number: 1 }
  )pb");
  NestedMessage expected_result = PARSE_TEXT_PROTO(R"pb(
    sub { devpath: "/phys/plus" number: 1 }
  )pb");

  // The transform will append /plus.
  auto func = [](absl::string_view devpath) -> absl::optional<std::string> {
    return std::string(devpath) + "/plus";
  };

  EXPECT_TRUE(TransformProtobufDevpaths(func, "sub.devpath", &message));
  EXPECT_THAT(message, EqualsProto(expected_result));
}

TEST(TransformDevpathsToDevpathsTest, FailsIfAnyTransformFails) {
  SimpleMessage message = PARSE_TEXT_PROTO(R"pb(
    devpath: "/phys/A"
    not_devpath: "/phys/B"
    also_devpath: "/phys/C/D"
    not_even_a_string: 4
  )pb");
  SimpleMessage first_result = message;

  // Fail on /phys/C/D, otherwise return the value unmodified.
  auto func = [](absl::string_view devpath) -> absl::optional<std::string> {
    if (devpath == "/phys/C/D")
      return absl::nullopt;
    else
      return std::string(devpath);
  };

  // First, only try transforming devpath. This should work and not actually
  // modify the message value.
  EXPECT_TRUE(TransformProtobufDevpaths(func, "devpath", &message));
  EXPECT_THAT(message, EqualsProto(first_result));
  // Now also include also_devpath. The function we're using should fail on
  // that and thus fail the entire transform.
  EXPECT_FALSE(
      TransformProtobufDevpaths(func, "devpath,also_devpath", &message));
}

TEST(TransformDevpathsToDevpathsTest, CannotTransformMissingFields) {
  SimpleMessage message = PARSE_TEXT_PROTO(R"pb(
    devpath: "/phys/A"
    not_devpath: "/phys/B"
    also_devpath: "/phys/C/D"
    not_even_a_string: 4
  )pb");

  // This should never be called.
  auto func = [](absl::string_view devpath) -> absl::optional<std::string> {
    ADD_FAILURE() << "transform function was incorrectly called";
    return absl::nullopt;
  };

  EXPECT_FALSE(TransformProtobufDevpaths(func, "not_a_field", &message));
}

TEST(TransformDevpathsToDevpathsTest, CannotTransformIntegers) {
  SimpleMessage message = PARSE_TEXT_PROTO(R"pb(
    devpath: "/phys/A"
    not_devpath: "/phys/B"
    also_devpath: "/phys/C/D"
    not_even_a_string: 4
  )pb");

  // This should never be called.
  auto func = [](absl::string_view devpath) -> absl::optional<std::string> {
    ADD_FAILURE() << "transform function was incorrectly called";
    return absl::nullopt;
  };

  EXPECT_FALSE(TransformProtobufDevpaths(func, "not_even_a_string", &message));
}

TEST(TransformDevpathsToDevpathsTest, SuccessfulTransformRepeatedFields) {
  RepeatedMessage message = PARSE_TEXT_PROTO(R"pb(
    multiple_devpaths: "/phys/B"
    multiple_devpaths: "/phys/C"
  )pb");

  RepeatedMessage expected = PARSE_TEXT_PROTO(R"pb(
    multiple_devpaths: "/phys/B/C"
    multiple_devpaths: "/phys/C/C"
  )pb");

  auto func = [](absl::string_view devpath) -> absl::optional<std::string> {
    std::string result(devpath);
    absl::StrAppend(&result, "/C");
    return result;
  };

  EXPECT_TRUE(TransformProtobufDevpaths(func, "multiple_devpaths", &message));
  EXPECT_THAT(message, EqualsProto(expected));
}

TEST(TransformDevpathsToDevpathsTest, ComplicatedMessage) {
  auto func =
      [](absl::string_view non_empty_value) -> absl::optional<std::string> {
    if (non_empty_value.empty()) {
      return absl::nullopt;
    }

    std::string result(non_empty_value);
    absl::StrAppend(&result, "aaa");
    return result;
  };

  {
    ComplicatedMessage message = PARSE_TEXT_PROTO(R"pb(
      bb: {
        multiple_devpaths: "b"
        cc: { devpath: "c1" }
        cc: { devpath: "c2" }
      }
      also_devpath: "a"
    )pb");
    ComplicatedMessage expected = PARSE_TEXT_PROTO(R"pb(
      bb: {
        multiple_devpaths: "b"
        cc: { devpath: "c1aaa" }
        cc: { devpath: "c2aaa" }
      }
      also_devpath: "a"
    )pb");

    EXPECT_TRUE(TransformProtobufDevpaths(func, "bb.cc.devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  {
    ComplicatedMessage message = PARSE_TEXT_PROTO(R"pb(
      also_devpath: "a"
    )pb");
    ComplicatedMessage expected = message;

    EXPECT_TRUE(TransformProtobufDevpaths(func, "bb.cc.devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  {
    ComplicatedMessage message = PARSE_TEXT_PROTO(R"pb(
      bb: { multiple_devpaths: "b1" multiple_devpaths: "b2" }
    )pb");
    ComplicatedMessage expected = PARSE_TEXT_PROTO(R"pb(
      bb: { multiple_devpaths: "b1aaa" multiple_devpaths: "b2aaa" }
    )pb");

    EXPECT_TRUE(
        TransformProtobufDevpaths(func, "bb.multiple_devpaths", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }
}

// Using the original ::proto2::util::FieldMaskUtil::GetFieldDescriptors
// wouldn’t pass this test, because it doesn’t dive into repeated sub-messages.
TEST(TransformDevpathsToDevpathsTest, RepeatedNestedMessageWithFullMask) {
  auto func =
      [](absl::string_view non_empty_value) -> absl::optional<std::string> {
    if (non_empty_value.empty()) {
      return absl::nullopt;
    }

    std::string result(non_empty_value);
    absl::StrAppend(&result, "aaa");
    return result;
  };

  GetEventLogEventsResponse message = PARSE_TEXT_PROTO(R"pb(
    events: { pci_error_data: { source: SOURCE_EVENTLOG devpath: "aaa" } }
    events: { pci_error_data: { source: SOURCE_EVENTLOG devpath: "bbb" } }
  )pb");
  GetEventLogEventsResponse expected = PARSE_TEXT_PROTO(R"pb(
    events: { pci_error_data: { source: SOURCE_EVENTLOG devpath: "aaaaaa" } }
    events: { pci_error_data: { source: SOURCE_EVENTLOG devpath: "bbbaaa" } }
  )pb");
  EXPECT_TRUE(TransformProtobufDevpaths(func, "events.pci_error_data.devpath",
                                        &message));
  EXPECT_THAT(message, EqualsProto(expected));
}

TEST(TransformDevpathsToDevpathsTest, OneofMessage) {
  auto func =
      [](absl::string_view non_empty_value) -> absl::optional<std::string> {
    if (non_empty_value.empty()) {
      return absl::nullopt;
    }

    std::string result(non_empty_value);
    absl::StrAppend(&result, "aaa");
    return result;
  };

  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb(devpath: "aaa")pb");
    OneofMessage expected = PARSE_TEXT_PROTO(R"pb(devpath: "aaaaaa")pb");
    EXPECT_TRUE(TransformProtobufDevpaths(func, "devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb()pb");
    OneofMessage expected = message;
    // False if the transform function says so.
    EXPECT_FALSE(TransformProtobufDevpaths(func, "devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb(not_devpath: 1)pb");
    OneofMessage expected = message;
    EXPECT_FALSE(TransformProtobufDevpaths(func, "not_devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  // Various kinds of nested oneof’s.
  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb(
      a { devpath: "aaa" }
    )pb");
    OneofMessage expected = PARSE_TEXT_PROTO(R"pb(
      a { devpath: "aaaaaa" }
    )pb");

    EXPECT_TRUE(TransformProtobufDevpaths(func, "a.devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb(
      b { devpath: "aaa" devpath: "bbb" }
    )pb");
    OneofMessage expected = PARSE_TEXT_PROTO(R"pb(
      b { devpath: "aaaaaa" devpath: "bbbaaa" }
    )pb");

    EXPECT_TRUE(TransformProtobufDevpaths(func, "b.devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb(
      c { devpath: "aaa" }
    )pb");
    OneofMessage expected = PARSE_TEXT_PROTO(R"pb(
      c { devpath: "aaaaaa" }
    )pb");

    EXPECT_TRUE(TransformProtobufDevpaths(func, "c.devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }
}

TEST(TransformDevpathsToDevpathsTest, IdentityOnOneof) {
  // Identify transformation on unset oneof’s should work as on normal fields.
  auto identity = [](absl::string_view value) -> absl::optional<std::string> {
    return std::string(value);
  };

  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb()pb");
    OneofMessage expected = message;
    EXPECT_TRUE(TransformProtobufDevpaths(identity, "devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb(not_devpath: 1)pb");
    OneofMessage expected = message;
    EXPECT_TRUE(TransformProtobufDevpaths(identity, "devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }

  {
    OneofMessage message = PARSE_TEXT_PROTO(R"pb(another_devpath: "aaa")pb");
    OneofMessage expected = message;
    EXPECT_TRUE(TransformProtobufDevpaths(identity, "devpath", &message));
    EXPECT_THAT(message, EqualsProto(expected));
  }
}

}  // namespace
}  // namespace ecclesia
