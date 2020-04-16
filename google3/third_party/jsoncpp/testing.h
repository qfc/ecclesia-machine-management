#ifndef THIRD_PARTY_JSONCPP_TESTING_H_
#define THIRD_PARTY_JSONCPP_TESTING_H_

#include "jsoncpp/testing_no_google3.h"  // iwyu pragma: export
#include "util/task/canonical_errors.h"
#include "util/task/statusor.h"
#include "jsoncpp/json.h"
#include "absl/strings/str_cat.h"

namespace Json {
namespace testing {

// Returns a Json::Value objected parsed from the given string. Returns
// FailedPrecondition Status on parse error.
inline ::util::StatusOr<Value> MakeJson(const absl::string_view s) {
  Json::Value v;
  auto reader = Json::Reader();
  if (!reader.parse(s.begin(), s.end(), v)) {
    return ::util::InvalidArgumentError(
        absl::StrCat("not valid JSON:\n", reader.getFormattedErrorMessages()));
  }
  return v;
}

}  // namespace testing
}  // namespace Json

#endif  // THIRD_PARTY_JSONCPP_TESTING_H_
