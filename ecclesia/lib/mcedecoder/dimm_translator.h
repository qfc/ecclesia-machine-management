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

#ifndef ECCLESIA_LIB_MCEDECODER_DIMM_TRANSLATOR_H_
#define ECCLESIA_LIB_MCEDECODER_DIMM_TRANSLATOR_H_

namespace mcedecoder {

// A DIMM slot can be uniquely identified by CPU socket ID, the IMC channel ID
// within CPU socket and the channel slot within a IMC channel.
struct DimmSlotId {
  int socket_id;
  int imc_channel;
  int channel_slot;
};

// Interface class of DIMM translator.
class DimmTranslatorInterface {
 public:
  virtual ~DimmTranslatorInterface() {}
  // Get Google Logical DIMM Number (GLDN) from the socket ID, IMC channel
  // number and channel slot ID. Set the gldn and return true if get GLDN
  // success. otherwise return false. Here imc_channel is per socket based
  // channel number, channel_slot is per imc_channel based slot number.
  virtual bool GetGLDN(int socket_id, int imc_channel, int channel_slot,
                       int *gldn) const = 0;

  // This method translates a GLDN to a DIMM slot. If the given GLDN is invalid,
  // return false; otherwise fill in the DIMM slot and return true.
  virtual bool GldnToSlot(int gldn, DimmSlotId *dimm_slot) const = 0;
};

}  // namespace mcedecoder

#endif  // ECCLESIA_LIB_MCEDECODER_DIMM_TRANSLATOR_H_
