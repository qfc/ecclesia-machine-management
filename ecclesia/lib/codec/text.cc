#include "lib/codec/text.h"

namespace ecclesia {

namespace {

// Lookup table for BCD-plus.
// Maps 0x0-0x9 to digits, 0xA to ' ', 0xB = '-' and 0xC = '.'
constexpr char kBcdPlusTable[] = "0123456789 -.";
// We subtract one because the actual c-style string above is null-terminated.
constexpr size_t kBcdUpperBound = sizeof(kBcdPlusTable) - 1;

}  // namespace

absl::Status ParseBcdPlus(
    absl::Span<const unsigned char> bytes, std::string *value) {
  value->clear();
  value->reserve(2 * bytes.size());
  for (uint8_t byte : bytes) {
    uint8_t lower_code = ExtractBits(byte, BitRange(3, 0));
    uint8_t upper_code = ExtractBits(byte, BitRange(7, 4));
    if (lower_code >= kBcdUpperBound || upper_code >= kBcdUpperBound) {
      return absl::InternalError(
          "BCD plus decoding failed! Data must be "
          "between 0h-Ch");
    } else {
      value->push_back(kBcdPlusTable[upper_code]);
      value->push_back(kBcdPlusTable[lower_code]);
    }
  }
  return absl::OkStatus();
}

absl::Status ParseSixBitAscii(
    absl::Span<const unsigned char> bytes, std::string *value) {
  uint32_t idx = 0;  // packed bytes starting index
  uint32_t buf = 0;  // buffer that holds 4 encoded bytes
  value->clear();
  for (uint8_t byte : bytes) {
    buf |= byte << (idx << 3);
    idx++;
    //  Packed 6-bit ASCII takes the 6-bit characters and packs them 4
    //  characters to every 3 bytes, with the first character in the least
    //  significant 6-bits of the first byte
    if (idx == 3) {
      value->push_back(0x20 + ExtractBits(buf, BitRange(5, 0)));
      value->push_back(0x20 + ExtractBits(buf, BitRange(11, 6)));
      value->push_back(0x20 + ExtractBits(buf, BitRange(17, 12)));
      value->push_back(0x20 + ExtractBits(buf, BitRange(23, 18)));
      buf = 0;
      idx = 0;
    }
  }
  // Handle the remaining byte_offset bytes if data length is not a
  // multiple of 3
  for (uint8_t bit_idx = 0; bit_idx < idx * 6; bit_idx += 6) {
    value->push_back(
        0x20 + ExtractBits(buf, BitRange(bit_idx + 5, bit_idx)));
  }
  return absl::OkStatus();
}

}  // namespace ecclesia
