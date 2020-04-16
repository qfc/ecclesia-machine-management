#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "absl/strings/substitute.h"
#include "jsoncpp/value.h"

class JsonValueFuzzer {
 public:
  JsonValueFuzzer(const uint8_t *data, size_t size)
      : data_provider_(data, size), remaining_size(size) {}
  void Start();

 private:
  std::string RandomLengthString();
  std::string RandomCommentString();
  Json::CommentPlacement RandomCommentPlacement();
  FuzzedDataProvider data_provider_;
  size_t remaining_size;
};

void JsonValueFuzzer::Start() {
  Json::Value fuzz_value(RandomLengthString());
  fuzz_value.setComment(RandomCommentString(), RandomCommentPlacement());
}

std::string JsonValueFuzzer::RandomLengthString() {
  std::string random_string =
      data_provider_.ConsumeRandomLengthString(remaining_size);
  remaining_size -= random_string.length();
  return random_string;
}

std::string JsonValueFuzzer::RandomCommentString() {
  std::string random_string = RandomLengthString();
  if (random_string.length() == 0) {
    return random_string;
  }
  // Comments must start with /
  random_string[0] = '/';
  return random_string;
}


Json::CommentPlacement JsonValueFuzzer::RandomCommentPlacement() {
  int random_value = data_provider_.ConsumeIntegralInRange<int>(
      0, Json::commentAfter);
  return static_cast<Json::CommentPlacement>(random_value);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  JsonValueFuzzer fuzzer(data, size);
  fuzzer.Start();
  return 0;
}
