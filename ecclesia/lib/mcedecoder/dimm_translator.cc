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

#include "lib/mcedecoder/dimm_translator.h"

namespace mcedecoder {
namespace {
constexpr int kIndusNumCpuSocket = 2;
constexpr int kIndusNumImcChannelsPerCpu = 6;
constexpr int kIndusNumDimmSlotPerImcChannel = 2;
// Map slot number to Google Logical DIMM Number (GLDN). Array index corresponds
// to slot number which can be calculated from socket_id, imc_channel and
// channel_slot. Array value represents the GLDN for the corresponding slot.
constexpr int kIndusSlotNumGldnMap[] = {11, 10, 9,  8,  7,  6,  0,  1,
                                        2,  3,  4,  5,  12, 13, 14, 15,
                                        16, 17, 23, 22, 21, 20, 19, 18};
}  // namespace

bool IndusDimmTranslator::GetGLDN(int socket_id, int imc_channel,
                                  int channel_slot, int *gldn) {
  if (socket_id < 0 || socket_id >= kIndusNumCpuSocket || imc_channel < 0 ||
      imc_channel >= kIndusNumImcChannelsPerCpu || channel_slot < 0 ||
      channel_slot >= kIndusNumDimmSlotPerImcChannel) {
    return false;
  }
  int slot_num =
      socket_id * kIndusNumImcChannelsPerCpu * kIndusNumDimmSlotPerImcChannel +
      imc_channel * kIndusNumDimmSlotPerImcChannel + channel_slot;
  *gldn = kIndusSlotNumGldnMap[slot_num];
  return true;
}

}  // namespace mcedecoder
