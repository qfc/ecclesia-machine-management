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

#include "ecclesia/magent/lib/nvme/smart_log_page.h"

#include <assert.h>

#include <cstdint>
#include <memory>
#include <string>

#include "absl/numeric/int128.h"
#include "ecclesia/lib/codec/endian.h"
#include "ecclesia/magent/lib/nvme/nvme_types.h"

namespace ecclesia {

std::unique_ptr<SmartLogPageInterface> SmartLogPage::Parse(
    const std::string &buf) {
  if (buf.size() != kSmartLogPageSize) return nullptr;
  return std::unique_ptr<SmartLogPage>(new SmartLogPage(buf));
}

SmartLogPage::SmartLogPage(const std::string &buf)
    : data_(buf),
      smart_log_(reinterpret_cast<SmartLogPageFormat *>(&data_[0])) {}

uint8_t SmartLogPage::critical_warning() const {
  return smart_log_->critical_warning;
}

uint16_t SmartLogPage::composite_temperature_kelvins() const {
  return LittleEndian::Load16(smart_log_->temperature);
}

uint8_t SmartLogPage::available_spare() const {
  return smart_log_->available_spare;
}

uint8_t SmartLogPage::available_spare_threshold() const {
  return smart_log_->available_spare_threshold;
}

uint8_t SmartLogPage::percent_used() const { return smart_log_->percent_used; }

uint8_t SmartLogPage::endurance_group_critical_warning() const {
  return smart_log_->endurance_cw;
}

absl::uint128 SmartLogPage::data_units_read() const {
  return LittleEndian::Load128(smart_log_->data_units_read);
}

absl::uint128 SmartLogPage::data_units_written() const {
  return LittleEndian::Load128(smart_log_->data_units_written);
}

absl::uint128 SmartLogPage::host_reads() const {
  return LittleEndian::Load128(smart_log_->host_reads);
}

absl::uint128 SmartLogPage::host_writes() const {
  return LittleEndian::Load128(smart_log_->host_writes);
}

absl::uint128 SmartLogPage::controller_busy_time_minutes() const {
  return LittleEndian::Load128(smart_log_->ctrl_busy_time);
}

absl::uint128 SmartLogPage::power_cycles() const {
  return LittleEndian::Load128(smart_log_->power_cycles);
}

absl::uint128 SmartLogPage::power_on_hours() const {
  return LittleEndian::Load128(smart_log_->power_on_hours);
}

absl::uint128 SmartLogPage::unsafe_shutdowns() const {
  return LittleEndian::Load128(smart_log_->unsafe_shutdowns);
}

absl::uint128 SmartLogPage::media_errors() const {
  return LittleEndian::Load128(smart_log_->media_errors);
}

absl::uint128 SmartLogPage::num_err_log_entries() const {
  return LittleEndian::Load128(smart_log_->num_err_log_entries);
}

uint32_t SmartLogPage::warning_temp_time_minutes() const {
  return LittleEndian::Load32(
      reinterpret_cast<const unsigned char *>(&smart_log_->warning_temp_time));
}

uint32_t SmartLogPage::critical_comp_time_minutes() const {
  return LittleEndian::Load32(
      reinterpret_cast<const unsigned char *>(&smart_log_->critical_comp_time));
}

uint16_t SmartLogPage::thermal_sensor_kelvins(int sensor) const {
  assert(sensor < kNumTemperatureSensors);
  return LittleEndian::Load16(reinterpret_cast<const unsigned char *>(
      &smart_log_->temp_sensor[sensor]));
}

uint32_t SmartLogPage::thermal_transition_count(int limit) const {
  assert(limit < kNumThermalManagementEntries);
  return LittleEndian::Load32(reinterpret_cast<const unsigned char *>(
      &smart_log_->thm_temp_trans_count[limit]));
}

uint32_t SmartLogPage::thermal_transition_minutes(int limit) const {
  assert(limit < kNumThermalManagementEntries);
  return LittleEndian::Load32(reinterpret_cast<const unsigned char *>(
      &smart_log_->thm_temp_total_time[limit]));
}

}  // namespace ecclesia
