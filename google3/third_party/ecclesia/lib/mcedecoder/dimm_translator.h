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

// Interface class of DIMM translator.
class DimmTranslatorInterface {
 public:
  virtual ~DimmTranslatorInterface() {}
  // Get Google Logical DIMM Number (GLDN) from the socket ID, IMC channel
  // number and channel slot ID. Set the gldn and return true if get GLDN
  // success. otherwise return false. Here imc_channel is per socket based
  // channel number, channel_slot is per imc_channel based slot number.
  virtual bool GetGLDN(int socket_id, int imc_channel, int channel_slot,
                       int *gldn) = 0;
};

// DIMM translator class for Indus server which has 2 CPU sockets, each CPU with
// 2 IMCs, each IMC with 3 IMC channels and each IMC channel with 2 slots.
class IndusDimmTranslator : public DimmTranslatorInterface {
 public:
  // imc_channel is 0~5, channel_slot is 0~1. If imc_id (0 or 1) and
  // imc_channel_id (0~2) are given instead, then the input imc_channel = imc_id
  // * 3 + imc_channel_id.
  bool GetGLDN(int socket_id, int imc_channel, int channel_slot,
               int *gldn) override;
};

}  // namespace mcedecoder

#endif  // ECCLESIA_LIB_MCEDECODER_DIMM_TRANSLATOR_H_
