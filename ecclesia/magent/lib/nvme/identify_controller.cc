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

#include "ecclesia/magent/lib/nvme/identify_controller.h"

#include <cstdint>
#include <memory>
#include <string>

#include "absl/base/macros.h"
#include "absl/numeric/int128.h"
#include "absl/strings/ascii.h"
#include "absl/strings/string_view.h"
#include "ecclesia/lib/codec/endian.h"
#include "ecclesia/magent/lib/nvme/nvme_types.h"

namespace ecclesia {

std::unique_ptr<IdentifyController> IdentifyController::Parse(
    const std::string &buf) {
  if (buf.size() != kIdentifyControllerSize) return nullptr;
  return std::unique_ptr<IdentifyController>(new IdentifyController(buf));
}

IdentifyController::IdentifyController(const std::string &buf)
    : data_(buf),
      identify_(reinterpret_cast<IdentifyControllerFormat *>(&data_[0])) {}

uint16_t IdentifyController::controller_id() const {
  return LittleEndian::Load16(identify_->cntlid);
}

uint16_t IdentifyController::vendor_id() const {
  return LittleEndian::Load16(identify_->vendor_id);
}

uint16_t IdentifyController::subsystem_vendor_id() const {
  return LittleEndian::Load16(identify_->subsystem_vendor_id);
}

std::string IdentifyController::serial_number() const {
  absl::string_view serial(identify_->serial_number,
                           ABSL_ARRAYSIZE(identify_->serial_number));
  return std::string(absl::StripAsciiWhitespace(serial));
}

std::string IdentifyController::model_number() const {
  absl::string_view model(identify_->model_number,
                          ABSL_ARRAYSIZE(identify_->model_number));
  return std::string(absl::StripAsciiWhitespace(model));
}

std::string IdentifyController::firmware_revision() const {
  absl::string_view version(identify_->firmware_revision,
                            ABSL_ARRAYSIZE(identify_->firmware_revision));
  return std::string(absl::StripAsciiWhitespace(version));
}

uint16_t IdentifyController::critical_temperature_threshold() const {
  return LittleEndian::Load16(identify_->cctemp);
}

uint16_t IdentifyController::warning_temperature_threshold() const {
  return LittleEndian::Load16(identify_->wctemp);
}

absl::uint128 IdentifyController::total_capacity() const {
  return LittleEndian::Load128(identify_->tnvmcap);
}

uint8_t IdentifyController::max_data_transfer_size() const {
  return identify_->mdts;
}

uint32_t IdentifyController::number_of_namespaces() const {
  return LittleEndian::Load32(identify_->nn);
}

uint16_t IdentifyController::max_error_log_page_entries() const {
  return static_cast<uint16_t>(identify_->elpe) + 1;
}

uint32_t IdentifyController::sanitize_capabilities() const {
  return LittleEndian::Load32(identify_->sanicap);
}

}  // namespace ecclesia
