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

#ifndef ECCLESIA_LIB_MCEDECODER_MCE_DECODE_H_
#define ECCLESIA_LIB_MCEDECODER_MCE_DECODE_H_

#include <memory>
#include <utility>

#include "lib/mcedecoder/cpu_topology.h"
#include "lib/mcedecoder/dimm_translator.h"
#include "lib/mcedecoder/mce_messages.h"

namespace mcedecoder {

// Interface class of Machine check exception (MCE) decoder.
class MceDecoderInterface {
 public:
  virtual ~MceDecoderInterface() {}

  // Decode MCE and fill in the input decoded_msg. If the event is not a MCE
  // or decoding failed, return false; otherwise return true.
  virtual bool DecodeMceMessage(const MceLogMessage& raw_msg,
                                MceDecodedMessage* decoded_msg) = 0;
};

enum class CpuVendor { kIntel, kAmd, kUnknown };

enum class CpuIdentifier { kSkylake, kCascadeLake, kUnknown };

// Machine check exception (MCE) decoder class.
class MceDecoder : public MceDecoderInterface {
 public:
  MceDecoder(CpuVendor cpu_vendor, CpuIdentifier cpu_identifier,
             std::unique_ptr<CpuTopologyInterface> cpu_topology,
             std::unique_ptr<DimmTranslatorInterface> dimm_translator)
      : cpu_vendor_(cpu_vendor),
        cpu_identifier_(cpu_identifier),
        cpu_topology_(std::move(cpu_topology)),
        dimm_translator_(std::move(dimm_translator)) {}

  bool DecodeMceMessage(const MceLogMessage& raw_msg,
                        MceDecodedMessage* decoded_msg) override;

 private:
  CpuVendor cpu_vendor_;
  CpuIdentifier cpu_identifier_;
  std::unique_ptr<CpuTopologyInterface> cpu_topology_;
  std::unique_ptr<DimmTranslatorInterface> dimm_translator_;
};

}  // namespace mcedecoder
#endif  // ECCLESIA_LIB_MCEDECODER_MCE_DECODE_H_
