#include <fuzzer/FuzzedDataProvider.h>

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "absl/strings/substitute.h"
#include "jsoncpp/value.h"
#include "jsoncpp/reader.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  FuzzedDataProvider data_provider(data, size);
  Json::Value fuzz_value(data_provider.ConsumeRandomLengthString(size));
  std::string fuzz_document = data_provider.ConsumeRemainingBytesAsString();
  Json::Reader reader;
  reader.parse(fuzz_document, fuzz_value);
  reader.pushError(fuzz_value, fuzz_document);
  return 0;
}
