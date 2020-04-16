// JsonCpp fuzzing wrapper to help with automated fuzz testing.

#include <stdint.h>

#include <climits>
#include <cstdio>
#include <memory>

#include "security/fuzzing/grammar_protos/json/json_proto_converter.h"
#include "testing/base/public/benchmark.h"
#include "absl/strings/substitute.h"
#include "jsoncpp/json.h"

DEFINE_PROTO_FUZZER(const fuzzing::proto::json::Json &j) {
  Json::Features features;

  if (j.has_allow_comments()) features.allowComments_ = j.allow_comments();
  if (j.has_strict_root()) features.strictRoot_ = j.strict_root();
  if (j.has_allow_dropped_null_placeholders())
    features.allowDroppedNullPlaceholders_ =
        j.allow_dropped_null_placeholders();
  if (j.has_allow_numeric_keys())
    features.allowNumericKeys_ = j.allow_numeric_keys();
  if (j.has_reject_dup_keys()) features.rejectDupKeys_ = j.reject_dup_keys();

  Json::Reader reader(features);
  Json::Value root;
  JsonProtoConverter jpc;
  std::string json_string = jpc.ProtoToString(j);
  bool successfully_parsed = reader.parse(
      reinterpret_cast<const char *>(json_string.data()),
      reinterpret_cast<const char *>(json_string.data() + json_string.size()),
      root,
      /*collectComments=*/true);
  if (!successfully_parsed) {
    return;
  }

  // Write and re-read json.
  Json::FastWriter writer;
  std::string output_json = writer.write(root);

  Json::Value root_again;
  successfully_parsed =
      reader.parse(output_json, root_again, /*collectComments=*/true);
  if (!successfully_parsed) {
    return;
  }

  // Run equality test.
  // Note: This actually causes the Json::Value tree to be traversed and all
  // the values to be dereferenced (until two of them are found not equal),
  // which is great for detecting memory corruption bugs when compiled with
  // AddressSanitizer. The result of the comparison is ignored, as it is
  // expected that both the original and the re-read version will differ from
  // time to time (e.g. due to floating point accuracy loss).
  testing::DoNotOptimize(root == root_again);

  std::string message = absl::Substitute(
      "{\"data\": $0}", Json::writeString(Json::StreamWriterBuilder(), root));
}
