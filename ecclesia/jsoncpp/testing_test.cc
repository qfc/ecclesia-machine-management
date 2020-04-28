// Copyright 2010 Google Inc. All Rights Reserved.
// Author: helder@google.com (Helder Suzuki)
//
// TODO(helder): Test EqualsJson explanation messages.

#include "jsoncpp/testing.h"

#include <string>

#include "testing/base/public/gunit.h"
#include "jsoncpp/value.h"
#include "util/task/codes.proto.h"
#include "testing/base/public/gmock.h"

namespace Json {

namespace testing {

namespace {

using ::testing::Not;

// Returns the reason why x matches, or doesn't match, m.
template <typename MatcherType, typename Value>
std::string Explain(const MatcherType& m, const Value& x) {
  ::testing::StringMatchResultListener listener;
  ::testing::ExplainMatchResult(m, x, &listener);
  return listener.str();
}

TEST(EqualsJsonTest, ExactlyEqualJSONValues) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"blue\", \"my\"],"
      "\"frequency\":120"
      "}";

  const std::string actual = expected;

  EXPECT_THAT(actual, EqualsJson(expected));
}

TEST(EqualsJsonTest, EqualJSONValues) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"blue\", \"my\"],"
      "\"frequency\":120"
      "}";

  const std::string actual =
      "{"
      "\"frequency\":120,"
      "\"parakeet\": [\"blue\", \"my\"]"
      "}";

  // expected and actual are equal JSON objects, they only differ in the order
  // of the attributes.
  EXPECT_THAT(actual, EqualsJson(expected));
}

TEST(EqualsJsonTest, ExactlyEqualJSONValuesNonObject) {
  const std::string expected = "10";
  const std::string actual = expected;

  EXPECT_THAT(actual, EqualsJson(expected));
}

TEST(EqualsJsonTest, UnEqualJSONValuesNonObject) {
  const std::string expected = "10";
  const std::string actual = "11";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("", Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, NestedUnEqualJSONValues) {
  const std::string expected = "[10]";
  const std::string actual = "[11]";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("with value at [0] which is 11 (expected 10)",
            Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, NestedUnEqualJSONValuesDifferentTypes) {
  const std::string expected = "[10]";
  const std::string actual = "[[11]]";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("with value at [0] which is of type array (expected integer)",
            Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, NestedUnEqualJSONValuesDifferentTypesReverse) {
  const std::string expected = "[[10]]";
  const std::string actual = "[11]";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("with value at [0] which is of type integer (expected array)",
            Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, DifferentArrayOrder) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}";

  const std::string actual =
      "{"
      "\"parakeet\": [\"blue\", \"my\"],"
      "\"frequency\":120"
      "}";

  // The only difference is the order inside the array "parakeet".
  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ(
      "with value at [\"parakeet\"][0] "
      "which is \"blue\" (expected \"my\")",
      Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, ArraysDifferentAtEndOrder) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}";

  const std::string actual =
      "{"
      "\"parakeet\": [\"my\", \"red\"],"
      "\"frequency\":120"
      "}";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ(
      "with value at [\"parakeet\"][1] "
      "which is \"red\" (expected \"blue\")",
      Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, ArrayDifferentArrayLength) {
  const std::string expected = "[\"my\", \"blue\"]";
  const std::string actual = "[\"my\"]";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("which is an array of length 1 (expected length 2)",
            Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, DifferentArrayLength) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}";

  const std::string actual =
      "{"
      "\"parakeet\": [\"my\"],"
      "\"frequency\":120"
      "}";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("with value at [\"parakeet\"] "
            "which is an array of length 1 (expected length 2)",
            Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, MissingObjectAttribute) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}";

  const std::string actual =
      "{"
      "\"parakeet\": [\"my\", \"blue\"]"
      "}";

  // "frequency" is missing in actual value.
  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("which has no member named \"frequency\"",
            Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, NestedMissingObjectAttribute) {
  const std::string expected =
      "[{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}]";

  const std::string actual =
      "[{"
      "\"parakeet\": [\"my\", \"blue\"]"
      "}]";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("with value at [0] which has no member named \"frequency\"",
            Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, ExtraObjectAttribute) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"my\", \"blue\"]"
      "}";

  const std::string actual =
      "{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ("which has an unexpected member named \"frequency\"",
            Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonTest, NestedExtraObjectAttribute) {
  const std::string expected =
      "[{"
      "\"parakeet\": [\"my\", \"blue\"]"
      "}]";

  const std::string actual =
      "[{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}]";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ(
      "with value at [0] "
      "which has an unexpected member named \"frequency\"",
      Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonDeathTest, InvalidExpectedJSON) {
  // expected value does not encode a valid JSON (missing comma in "parakeet")
  const std::string expected =
      "{"
      "\"parakeet\": [\"my\" \"blue\"],"
      "\"frequency\":120"
      "}";

  const std::string actual =
      "{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}";

  EXPECT_DEATH({ EXPECT_THAT(actual, Not(EqualsJson(expected))); },
               ".*Expected value is not valid JSON.*");
}

TEST(EqualsJsonTest, InvalidActualJSON) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\":120"
      "}";

  // actual value does not encode a valid JSON (missing colon after frequency)
  const std::string actual =
      "{"
      "\"parakeet\": [\"my\", \"blue\"],"
      "\"frequency\"120"
      "}";

  EXPECT_THAT(actual, Not(EqualsJson(expected)));
  EXPECT_EQ(
      "which is not valid JSON:\n"
      "* Line 1, Column 40\n"
      "  Missing ':' after object member name\n",
      Explain(EqualsJson(expected), actual));
}

TEST(EqualsJsonDeathTest, InvalidActualAndExpectedJSON) {
  // Both actual and expected values does not encode valid JSON, the equality
  // test must fail even if both strings are equal.

  const std::string invalid =
      "{"
      "\"parakeet\": [\"my\", \"blue\","
      "\"frequency\":120"
      "}";
  // missing ] in "parakeet"

  EXPECT_DEATH({ EXPECT_THAT(invalid, Not(EqualsJson(invalid))); },
               ".*Expected value is not valid JSON.*");
}

TEST(EqualsJsonDeathTest, MissingNulls) {
  const std::string expected =
      "{"
      "\"parakeet\": [\"my\", , , \"blue\"],"
      "\"frequency\":120"
      "}";

  // actual value encodes nulls.
  const std::string actual =
      "{"
      "\"parakeet\": [\"my\", null, null, \"blue\"],"
      "\"frequency\":120"
      "}";

  EXPECT_DEATH({ EXPECT_THAT(actual, Not(EqualsJson(expected))); },
               ".*Expected value is not valid JSON.*");

  // The permissive reader should be able to handle this gracefully.
  EXPECT_THAT(actual, EqualsJsonPermissive(expected));
}

TEST(EqualsJsonDeathTest, NumericObjectKeys) {
  const std::string expected =
      "{"
      "1: [\"my\", \"blue\"],"
      "1.2345:120"
      "}";

  // actual value encodes nulls.
  const std::string actual =
      "{"
      "\"1\": [\"my\", \"blue\"],"
      "\"1.2345\":120"
      "}";

  EXPECT_DEATH({ EXPECT_THAT(actual, Not(EqualsJson(expected))); },
               ".*Expected value is not valid JSON.*");

  // The permissive reader should be able to handle this gracefully.
  EXPECT_THAT(actual, EqualsJsonPermissive(expected));
}

TEST(EqualsJsonTest, StringExpectedJsonValueActual) {
  const std::string expected = R"(
{
  "parakeet": ["blue", "my"],
  "frequency": 120
}
)";

  Value actual;
  actual["parakeet"].append("blue");
  actual["parakeet"].append("my");
  actual["frequency"] = 120;

  EXPECT_THAT(actual, EqualsJson(expected));
}

TEST(EqualsJsonTest, JsonValueExpectedStringActual) {
  Value expected;
  expected["parakeet"].append("blue");
  expected["parakeet"].append("my");
  expected["frequency"] = 120;

  const std::string actual = R"(
{
  "parakeet": ["blue", "my"],
  "frequency": 120
}
)";

  EXPECT_THAT(actual, EqualsJson(expected));
}

TEST(EqualsJsonTest, JsonValueExpectedAndActual) {
  Value expected;
  expected["parakeet"].append("blue");
  expected["parakeet"].append("my");
  expected["frequency"] = 120;

  const Value actual = expected;

  EXPECT_THAT(actual, EqualsJson(expected));
}

TEST(EqualsJsonTest, CstringExpectedAndActual) {
  const char* expected = R"(
{
  "parakeet": ["blue", "my"],
  "frequency": 120
}
)";

  const char* actual = expected;

  EXPECT_THAT(actual, EqualsJson(expected));
}

TEST(MakeJsonTest, ParseFailureDeath) {
  EXPECT_DEATH(
      {
        MakeJsonOrDie(R"(
{
  []
}
)");
      },
      ".*");
}

TEST(MakeJsonTest, ParseFailureError) {
  EXPECT_THAT(MakeJson(R"(
{
  []
}
)"),
              ::testing::status::StatusIs(util::error::INVALID_ARGUMENT));
}

TEST(MakeJsonTest, ParseSuccess) {
  Value expected;
  expected["parakeet"].append("blue");
  expected["parakeet"].append("my");
  expected["frequency"] = 120;

  EXPECT_THAT(MakeJson(R"(
{
  "parakeet": ["blue", "my"],
  "frequency": 120
}
)"),
              ::testing::status::IsOkAndHolds(expected));
}

}  // namespace

}  // namespace testing

}  // namespace Json
