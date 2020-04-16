#include <stdint.h>
#include <cstdint>
#include <memory>
#include <string>
#include "jsoncpp/json.h"
#include "jsoncpp/config.h"

namespace Json {
class Exception;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  Json::CharReaderBuilder builder;

  if (size < sizeof(uint32_t)) {
    return 0;
  }

  uint32_t hash_settings = *(const uint32_t*)data;
  data += sizeof(uint32_t);
  size -= sizeof(uint32_t);

  builder.settings_["failIfExtra"] = hash_settings & (1 << 0);
  builder.settings_["allowComments"] = hash_settings & (1 << 1);
  builder.settings_["strictRoot"] = hash_settings & (1 << 2);
  builder.settings_["allowDroppedNullPlaceholders"] = hash_settings & (1 << 3);
  builder.settings_["allowNumericKeys"] = hash_settings & (1 << 4);
  builder.settings_["allowSingleQuotes"] = hash_settings & (1 << 5);
  builder.settings_["failIfExtra"] = hash_settings & (1 << 6);
  builder.settings_["rejectDupKeys"] = hash_settings & (1 << 7);
  builder.settings_["allowSpecialFloats"] = hash_settings & (1 << 8);
  builder.settings_["collectComments"] = hash_settings & (1 << 9);

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

  Json::Value root;
  const char* data_str = reinterpret_cast<const char*>(data);
  try {
    reader->parse(data_str, data_str + size, &root, nullptr);
  } catch (Json::Exception const&) {
  }
  // Whether it succeeded or not doesn't matter.
  return 0;
}
