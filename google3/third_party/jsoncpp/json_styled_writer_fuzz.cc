#include <sstream>
#include <string>

#include "jsoncpp/config.h"
#include "jsoncpp/features.h"
#include "jsoncpp/reader.h"
#include "jsoncpp/value.h"
#include "jsoncpp/writer.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string json_string(reinterpret_cast<const char *>(data), size);
  Json::Reader reader(Json::Features::strictMode());
  Json::Value value;
  const bool success = reader.parse(json_string, value, false);
  if (!success) {
    return 0;
  }

  // Write with StyledWriter
  Json::StyledWriter styled_writer;
  styled_writer.write(value);

  // Write with StyledStreamWriter
  Json::StyledStreamWriter styled_stream_writer;
  JSONCPP_OSTRINGSTREAM sstream;
  styled_stream_writer.write(sstream, value);
  return 0;
}
