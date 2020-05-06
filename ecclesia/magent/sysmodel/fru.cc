/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "magent/sysmodel/fru.h"

#include <stddef.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "magent/lib/eeprom/smbus_eeprom.h"
#include "magent/lib/fru/fru.h"

namespace ecclesia {

namespace {

constexpr int kFruCommonHeaderSize = 8;
constexpr int kFruBoardInfoAreaSizeIndex = 9;

// Given an input string, returns a new string that contains only the portion
// of the string up to (but not including) the first NUL character. If the
// input string contains no NUL characters than the returned value will be
// equal to the input.
//
// The implementation is intended to be equivalent to:
//    return s.c_str();
// but we want to avoid this because many linting tools will (correctly) flag
// string -> char* -> string conversions as possible bugs.

std::string StringUpToNul(std::string s) { return s.substr(0, s.find('\0')); }

absl::Status ValidateFruCommonHeader(
    absl::Span<const unsigned char> common_header) {
  uint8_t sum = 0;

  for (size_t i = 0; i < common_header.size(); i++) {
    sum += common_header[i];
  }

  if (sum != 0) {
    return absl::InternalError(absl::StrFormat(
        "Fru common header has invalid checksum(should be 0, is %u).", sum));
  }

  return absl::OkStatus();
}

absl::Status GetBoardInfoAreaSize(SmbusEeprom2ByteAddr &eeprom, size_t *size) {
  std::vector<unsigned char> value(1);
  absl::Status status;

  if (1 != eeprom.ReadBytes(kFruBoardInfoAreaSizeIndex,
                            absl::MakeSpan(value.data(), 1))) {
    return absl::InternalError("Failed to read fru BoardInfoArea size.");
  }

  // size is in multple of 8 bytes
  *size = value[0] << 3;

  return status;
}

absl::Status GetBoardInfo(const SmbusEeprom2ByteAddr::Option &eeprom_option,
                          FruInfo &info) {
  absl::Status status;

  SmbusEeprom2ByteAddr eeprom(eeprom_option);

  size_t board_info_area_size;
  status = GetBoardInfoAreaSize(eeprom, &board_info_area_size);
  if (!status.ok()) {
    return status;
  }

  // We will read the common header(8 bytes) and BoardInfoArea
  std::vector<unsigned char> fru_data(
      board_info_area_size + kFruCommonHeaderSize, 0);

  // We read from offset 0, this will have common header
  eeprom.ReadBytes(0, absl::MakeSpan(fru_data.data(), fru_data.size()));

  // Validate fru common header
  status = ValidateFruCommonHeader(
      absl::MakeSpan(fru_data.data(), kFruCommonHeaderSize));
  if (!status.ok()) {
    return status;
  }

  VectorFruImageSource fru_image(absl::MakeSpan(fru_data));
  BoardInfoArea board_info;

  // BoardInfoArea is right after common header,
  // starts from offset: 0 + kFruCommonHeaderSize
  board_info.FillFromImage(fru_image, 0 + kFruCommonHeaderSize);

  info = {
      .product_name =
          StringUpToNul(board_info.product_name().GetDataAsString()),
      .manufacturer =
          StringUpToNul(board_info.manufacturer().GetDataAsString()),
      .serial_number =
          StringUpToNul(board_info.serial_number().GetDataAsString()),
      .part_number = StringUpToNul(board_info.part_number().GetDataAsString())};

  return status;
}

}  // namespace

SysmodelFru::SysmodelFru(FruInfo fru_info) : fru_info_(std::move(fru_info)) {}

absl::string_view SysmodelFru::GetManufacturer() const {
  return fru_info_.manufacturer;
}
absl::string_view SysmodelFru::GetSerialNumber() const {
  return fru_info_.serial_number;
}
absl::string_view SysmodelFru::GetPartNumber() const {
  return fru_info_.part_number;
}

absl::flat_hash_map<std::string, SysmodelFru> CreateFrus(
    absl::Span<SmbusEeprom2ByteAddr::Option> eeprom_options) {
  absl::flat_hash_map<std::string, SysmodelFru> frus_map;

  for (SmbusEeprom2ByteAddr::Option &eeprom_option : eeprom_options) {
    if (eeprom_option.name == "motherboard") {
      FruInfo info;
      absl::Status status = GetBoardInfo(eeprom_option, info);
      if (!status.ok()) {
        std::cerr << "GetBoardInfo failed: " << status.message() << '\n';
        continue;
      }
      frus_map.emplace(eeprom_option.name, SysmodelFru(std::move(info)));
    }
  }

  return frus_map;
}

}  // namespace ecclesia
