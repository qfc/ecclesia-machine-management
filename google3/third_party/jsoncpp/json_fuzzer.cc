// JsonCpp fuzzing wrapper to help with automated fuzz testing.

#include <stdint.h>
#include <climits>
#include <cstdio>
#include <memory>
#include "testing/base/public/benchmark.h"
#include "jsoncpp/json.h"


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < sizeof(uint32_t)) {
    return 0;
  }

  uint32_t hash_settings = *(const uint32_t*)data;
  data += sizeof(uint32_t);

  Json::Features features;
  features.allowComments_ = hash_settings & (1 << 0);
  features.strictRoot_ = hash_settings & (1 << 1);
  features.allowDroppedNullPlaceholders_ = hash_settings & (1 << 2);
  features.allowNumericKeys_ = hash_settings & (1 << 3);

  Json::Reader reader(features);
  Json::Value root;
  bool res = reader.parse(reinterpret_cast<const char*>(data),
                          reinterpret_cast<const char*>(data + size),
                          root, /*collectComments=*/true);
  if (!res) {
    return 0;
  }

  // Write and re-read json.
  Json::FastWriter writer;
  std::string output_json = writer.write(root);

  Json::Value root_again;
  res = reader.parse(output_json, root_again, /*collectComments=*/true);
  if (!res) {
    return 0;
  }

  // Run equality test.
  // Note: This actually causes the Json::Value tree to be traversed and all
  // the values to be dereferenced (until two of them are found not equal),
  // which is great for detecting memory corruption bugs when compiled with
  // AddressSanitizer. The result of the comparison is ignored, as it is
  // expected that both the original and the re-read version will differ from
  // time to time (e.g. due to floating point accuracy loss).
  testing::DoNotOptimize(root == root_again);

  return 0;
}
