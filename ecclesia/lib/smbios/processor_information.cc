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

#include "ecclesia/lib/smbios/processor_information.h"

#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "ecclesia/lib/smbios/structures.emb.h"
#include "runtime/cpp/emboss_prelude.h"

namespace ecclesia {

namespace {

// From the SMBIOS Spec.
enum ProcessorFamily {
  OTHER = 0x1,
  UNKNOWN = 0x2,
  INTEL_XEON = 0xB3,
  ARM_V8 = 0x101
};

}  // namespace

CpuSignature ProcessorInformation::GetSignaturex86() const {
  CpuSignature signature;

  auto view = this->GetMessageView();
  signature.vendor = this->GetString(view.manufacturer_snum().Read());
  signature.type = view.processor_id_x86().processor_type().Read();
  // From Intel's instruction set reference for CPUID
  signature.family = (view.processor_id_x86().family_id_ext().Read() << 4) +
                     view.processor_id_x86().family_id().Read();
  signature.model = (view.processor_id_x86().model_ext().Read() << 4) +
                    view.processor_id_x86().model().Read();
  signature.stepping = view.processor_id_x86().stepping_id().Read();

  return signature;
}

absl::optional<CpuSignature> ProcessorInformation::GetSignature() const {
  if (IsIntelProcessor()) {
    return this->GetSignaturex86();
  }
  return absl::nullopt;
}

bool ProcessorInformation::IsIntelProcessor() const {
  return this->GetProcessorFamily() == INTEL_XEON;
}

}  // namespace ecclesia
