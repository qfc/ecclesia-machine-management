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

#include "ecclesia/magent/lib/nvme/sanitize_log_page.h"

#include <cstdint>
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ecclesia/lib/codec/endian.h"
#include "ecclesia/magent/lib/nvme/nvme_types.h"

namespace ecclesia {

absl::StatusOr<std::unique_ptr<SanitizeLogPageInterface>>
SanitizeLogPage::Parse(const std::string &buf) {
  if (buf.size() != kSanitizeLogPageSize) {
    return absl::InternalError(
        "The sanitize log page size must be equal to kSanitizeLogPageSize.");
  }
  return std::unique_ptr<SanitizeLogPageInterface>(new SanitizeLogPage(buf));
}

SanitizeLogPage::SanitizeLogPage(const std::string &buf)
    : data_(buf),
      sanitize_log_(reinterpret_cast<SanitizeLogPageFormat *>(&data_[0])) {}

uint16_t SanitizeLogPage::progress() const {
  return LittleEndian::Load16(
      reinterpret_cast<const unsigned char *>(&sanitize_log_->progress));
}

uint8_t SanitizeLogPage::recent_sanitize_status() const {
  return sanitize_log_->recent_sanitize_status;
}

uint8_t SanitizeLogPage::completed_overwrite_passes() const {
  return sanitize_log_->overwrite_passes;
}

bool SanitizeLogPage::written_after_sanitize() const {
  return !sanitize_log_->global_data_erased;
}

uint8_t SanitizeLogPage::sanitize_action() const {
  return sanitize_log_->cdw10.sanact;
}

bool SanitizeLogPage::allow_unrestricted_sanitize_exit() const {
  return sanitize_log_->cdw10.ause;
}

uint8_t SanitizeLogPage::overwrite_pass_count() const {
  return sanitize_log_->cdw10.owpass;
}

bool SanitizeLogPage::overwrite_invert_pattern() const {
  return sanitize_log_->cdw10.oipbp;
}

bool SanitizeLogPage::no_dealloc() const {
  return sanitize_log_->cdw10.no_dealloc;
}

uint32_t SanitizeLogPage::estimate_overwrite_time() const {
  return LittleEndian::Load32(reinterpret_cast<const unsigned char *>(
      &sanitize_log_->estimate_overwrite_time));
}

uint32_t SanitizeLogPage::estimate_block_erase_time() const {
  return LittleEndian::Load32(reinterpret_cast<const unsigned char *>(
      &sanitize_log_->estimate_block_erase_time));
}

uint32_t SanitizeLogPage::estimate_crypto_erase_time() const {
  return LittleEndian::Load32(reinterpret_cast<const unsigned char *>(
      &sanitize_log_->estimate_crypto_erase_time));
}

}  // namespace ecclesia
