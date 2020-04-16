// Copyright 2010 Google Inc. All Rights Reserved.
// Author: helder@google.com (Helder Suzuki)
//
// This is a very simple equality matcher for Json::Value and JSON strings that
// are parseable by Reader.parse().
//
// Quick usage reference with EXPECT_THAT and ASSERT_THAT:
// EXPECT_THAT(actual, EqualsJson(expected))
// ASSERT_THAT(actual, EqualsJson(expected))
//
// Caveats: it doesn't give detailed info on the difference (e.g. missing
//          values in array). It also doesn't handle floating point value
//          comparison well.
//
// Special thanks to vladl, sfiera and wan for helping put this together.

#ifndef THIRD_PARTY_JSONCPP_TESTING_NO_GOOGLE3_H_
#define THIRD_PARTY_JSONCPP_TESTING_NO_GOOGLE3_H_

#include <ostream>
#include <sstream>
#include <string>

#include "base/logging.h"
#include "testing/base/public/gmock.h"
#include "absl/strings/string_view.h"
#include "jsoncpp/json.h"

namespace Json {

inline void PrintTo(const Value& value, std::ostream* os) {
  auto out = StyledWriter().write(value);
  out.pop_back();  // StyledWriter always adds a newline.
  *os << out;
}

namespace testing {

// Returns a Json::Value object parsed from the given string. CHECK-fails on
// parse error.
inline Value MakeJsonOrDie(const absl::string_view s) {
  Json::Value v;
  CHECK(Json::Reader().parse(std::string(s), v)) << s;
  return v;
}

namespace internal {

inline std::string ExplainFailure(const Value& actual, const Value& expected,
                                  std::ostream* path) {
  if (expected.isArray() && actual.isArray()) {
    ArrayIndex actual_size = actual.size();
    if (actual_size != expected.size())
      return "which is an array of length " + std::to_string(actual_size) +
             " (expected length " + std::to_string(expected.size()) + ")";
    for (ArrayIndex index = 0; index < actual_size; ++index) {
      if (actual[index] == expected[index]) continue;
      *path << "[" << ::testing::PrintToString(index) << "]";
      return ExplainFailure(actual[index], expected[index], path);
    }
  } else if (expected.isObject() && actual.isObject()) {
    for (const auto& name : expected.getMemberNames()) {
      if (!actual.isMember(name))
        return "which has no member named " + ::testing::PrintToString(name);
      if (actual[name] == expected[name]) continue;
      *path << "[" << ::testing::PrintToString(name) << "]";
      return ExplainFailure(actual[name], expected[name], path);
    }
    for (const auto& name : actual.getMemberNames()) {
      if (!expected.isMember(name))
        return "which has an unexpected member named " +
               ::testing::PrintToString(name);
    }
  }
  return "which is " + ::testing::PrintToString(actual) + " (expected " +
         ::testing::PrintToString(expected) + ")";
}

class EqualsJsonMatcher {
 public:
  explicit EqualsJsonMatcher(const Features& features,
                             const Value& expected_json)
      : features_(features), expected_json_(expected_json) {}

  bool MatchAndExplain(const absl::string_view actual,
                       ::testing::MatchResultListener* listener) const {
    Reader reader(features_);
    Value actual_json;
    if (!reader.parse(std::string(actual), actual_json)) {
      *listener << "which is not valid JSON:\n"
                << reader.getFormattedErrorMessages();
      return false;
    }
    return MatchAndExplain(actual_json, listener);
  }

  // These string-like overloads are necessary to avoid ambiguous implicit
  // conversion to absl::string_view and Json::Value.
#ifdef HAS_GLOBAL_STRING
  bool MatchAndExplain(const string& actual,
                       ::testing::MatchResultListener* listener) const {
    return MatchAndExplain(absl::string_view(actual), listener);
  }
#endif
  bool MatchAndExplain(const std::string& actual,
                       ::testing::MatchResultListener* listener) const {
    return MatchAndExplain(absl::string_view(actual), listener);
  }
  bool MatchAndExplain(const char* actual,
                       ::testing::MatchResultListener* listener) const {
    return MatchAndExplain(absl::string_view(actual), listener);
  }

  bool MatchAndExplain(const Value& actual_json,
                       ::testing::MatchResultListener* listener) const {
    if (actual_json == expected_json_) return true;
    if (actual_json.type() != expected_json_.type()) return false;
    if (!expected_json_.isObject() && !expected_json_.isArray()) return false;
    std::ostringstream path;
    std::string explanation =
        ExplainFailure(actual_json, expected_json_, &path);
    std::string path_str = path.str();
    if (!path_str.empty()) *listener << "with value at " << path_str << " ";
    *listener << explanation;
    return false;
  }

  void DescribeTo(std::ostream* os) const {
    *os << "equals JSON ";
    PrintTo(expected_json_, os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "doesn't equal JSON ";
    PrintTo(expected_json_, os);
  }

 private:
  Features features_;
  Value expected_json_;
};

}  // namespace internal

inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher> EqualsJson(
    const absl::string_view expected) {
  Reader reader;
  Value expected_json;
  CHECK(reader.parse(std::string(expected), expected_json))
      << "Expected value is not valid JSON:\n"
      << reader.getFormattedErrorMessages() << "\n"
      << expected;
  return ::testing::MakePolymorphicMatcher(
      internal::EqualsJsonMatcher(Features(), expected_json));
}

#ifdef HAS_GLOBAL_STRING
inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher> EqualsJson(
    const string& expected) {
  return EqualsJson(absl::string_view(expected));
}
#endif

inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher> EqualsJson(
    const std::string& expected) {
  return EqualsJson(absl::string_view(expected));
}

inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher> EqualsJson(
    const char* expected) {
  return EqualsJson(absl::string_view(expected));
}

inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher> EqualsJson(
    const Json::Value& expected_json) {
  return ::testing::MakePolymorphicMatcher(
      internal::EqualsJsonMatcher(Features(), expected_json));
}

// These matchers are similar to the EqualsJson matchers but use a more
// permissive reader for parsing.
inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher>
EqualsJsonPermissive(const absl::string_view expected) {
  Features features;
  features.allowDroppedNullPlaceholders_ = true;
  features.allowNumericKeys_ = true;

  Reader reader(features);
  Value expected_json;
  CHECK(reader.parse(std::string(expected), expected_json))
      << "Expected value is not valid JSON:\n"
      << reader.getFormattedErrorMessages() << "\n"
      << expected;

  return ::testing::MakePolymorphicMatcher(
      internal::EqualsJsonMatcher(features, expected_json));
}

#ifdef HAS_GLOBAL_STRING
inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher>
EqualsJsonPermissive(const string& expected) {
  return EqualsJsonPermissive(absl::string_view(expected));
}
#endif

inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher>
EqualsJsonPermissive(const std::string& expected) {
  return EqualsJsonPermissive(absl::string_view(expected));
}

inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher>
EqualsJsonPermissive(const char* expected) {
  return EqualsJsonPermissive(absl::string_view(expected));
}

inline ::testing::PolymorphicMatcher<internal::EqualsJsonMatcher>
EqualsJsonPermissive(const Json::Value& expected_json) {
  Features features;
  features.allowDroppedNullPlaceholders_ = true;
  features.allowNumericKeys_ = true;
  return ::testing::MakePolymorphicMatcher(
      internal::EqualsJsonMatcher(features, expected_json));
}

}  // namespace testing
}  // namespace Json

#endif  // THIRD_PARTY_JSONCPP_TESTING_NO_GOOGLE3_H_
