#include "lib/codec/text.h"

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"

namespace ecclesia {
namespace {

TEST(TextDecode, DecodeValidBCD) {
  static constexpr char kBcdTranslated[] = "42-421 ...";
  static constexpr uint8_t kValidBcdString[] = {0x42, 0xB4, 0x21, 0xAC, 0xCC};

  std::string value;
  auto status = ParseBcdPlus(kValidBcdString, &value);
  ASSERT_OK(status);
  EXPECT_EQ(kBcdTranslated, value);
}

TEST(TextDecode, DecodeValidSixBit) {
  static constexpr char k6BitTranslated[] = "WHY?";
  static constexpr char k6BitTranslated_2[] = "WHY?F";
  static constexpr uint8_t kValidSixBitAscii[] = {0x37, 0x9A, 0x7F};
  static constexpr uint8_t kValidSixBitAscii_2[] = {0x37, 0x9A, 0x7F, 0x66};

  std::string value;
  auto status = ParseSixBitAscii(kValidSixBitAscii, &value);
  ASSERT_OK(status);
  EXPECT_EQ(value, std::string(k6BitTranslated));

  status = ParseSixBitAscii(kValidSixBitAscii_2, &value);
  ASSERT_OK(status);
  EXPECT_EQ(value, std::string(k6BitTranslated_2));
}

}  // namespace
}  // namespace ecclesia
