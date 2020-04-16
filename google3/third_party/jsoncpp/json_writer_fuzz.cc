#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "absl/strings/substitute.h"
#include "jsoncpp/value.h"
#include "jsoncpp/writer.h"

namespace {

void Fuzz_CreateWrappedMessage(const std::string &fuzz_json_value) {
  Json::Value json_data(fuzz_json_value);
  std::string message = absl::Substitute(
      "{\"data\": $0}",
      Json::writeString(Json::StreamWriterBuilder(), json_data));
}

}  // namespace


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::basic_string<char> fuzz_json_value(reinterpret_cast<const char *>(data),
                                          size);
  Fuzz_CreateWrappedMessage(fuzz_json_value);

  Json::valueToQuotedString(fuzz_json_value.c_str());

  return 0;
}
