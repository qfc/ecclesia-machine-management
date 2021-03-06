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

#include "ecclesia/lib/mcedecoder/interlaken/dimm_translator.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ecclesia/lib/mcedecoder/dimm_translator.h"

namespace ecclesia {
namespace {

constexpr int kInterlakenNumCpuSocket = 2;
constexpr int kInterlakenNumImcChannelsPerCpu = 6;
constexpr int kInterlakenNumDimmSlotPerImcChannel = 2;

// Map slot number to Google Logical DIMM Number (GLDN). Array index corresponds
// to slot number which can be calculated from socket_id, imc_channel and
// channel_slot. Array value represents the GLDN for the corresponding slot.
constexpr int kInterlakenSlotNumGldnMap[] = {11, 10, 9,  8,  7,  6,  0,  1,
                                             2,  3,  4,  5,  12, 13, 14, 15,
                                             16, 17, 23, 22, 21, 20, 19, 18};
}  // namespace

absl::StatusOr<int> InterlakenDimmTranslator::GetGldn(
    const DimmSlotId &dimm_slot) const {
  if (dimm_slot.socket_id < 0 ||
      dimm_slot.socket_id >= kInterlakenNumCpuSocket ||
      dimm_slot.imc_channel < 0 ||
      dimm_slot.imc_channel >= kInterlakenNumImcChannelsPerCpu ||
      dimm_slot.channel_slot < 0 ||
      dimm_slot.channel_slot >= kInterlakenNumDimmSlotPerImcChannel) {
    return absl::NotFoundError("dimm slot doesn't not match any known gldn");
  }
  int slot_num = dimm_slot.socket_id * kInterlakenNumImcChannelsPerCpu *
                     kInterlakenNumDimmSlotPerImcChannel +
                 dimm_slot.imc_channel * kInterlakenNumDimmSlotPerImcChannel +
                 dimm_slot.channel_slot;
  return kInterlakenSlotNumGldnMap[slot_num];
}

absl::StatusOr<DimmSlotId> InterlakenDimmTranslator::GldnToSlot(
    int gldn) const {
  int num_slots = kInterlakenNumCpuSocket * kInterlakenNumImcChannelsPerCpu *
                  kInterlakenNumDimmSlotPerImcChannel;
  if (gldn < 0 || gldn >= num_slots) {
    return absl::NotFoundError("gldn does not match any known dimm slot");
  }
  for (int slot_idx = 0; slot_idx < num_slots; ++slot_idx) {
    if (kInterlakenSlotNumGldnMap[slot_idx] == gldn) {
      int num_dimm_slot_per_cpu =
          kInterlakenNumImcChannelsPerCpu * kInterlakenNumDimmSlotPerImcChannel;
      return DimmSlotId{
          .socket_id = slot_idx / num_dimm_slot_per_cpu,
          .imc_channel = (slot_idx % num_dimm_slot_per_cpu) /
                         kInterlakenNumDimmSlotPerImcChannel,
          .channel_slot = slot_idx % kInterlakenNumDimmSlotPerImcChannel,
      };
    }
  }
  return absl::NotFoundError("gldn does not match any known dimm slot");
}

}  // namespace ecclesia
