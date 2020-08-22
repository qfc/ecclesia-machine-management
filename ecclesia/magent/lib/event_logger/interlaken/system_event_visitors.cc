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

#include "ecclesia/magent/lib/event_logger/interlaken/system_event_visitors.h"

#include <memory>
#include <utility>

#include "absl/time/time.h"
#include "ecclesia/lib/mcedecoder/cpu_topology.h"
#include "ecclesia/lib/mcedecoder/interlaken/dimm_translator.h"
#include "ecclesia/lib/mcedecoder/mce_decode.h"
#include "ecclesia/magent/lib/event_logger/system_event_visitors.h"

namespace ecclesia {

namespace {

std::unique_ptr<mcedecoder::MceDecoder> CreateInterlakenMceDecoder(
    std::unique_ptr<mcedecoder::CpuTopologyInterface> cpu_topology) {
  mcedecoder::CpuVendor vendor = mcedecoder::CpuVendor::kIntel;
  mcedecoder::CpuIdentifier identifier = mcedecoder::CpuIdentifier::kSkylake;
  return absl::make_unique<mcedecoder::MceDecoder>(
      vendor, identifier, std::move(cpu_topology),
      absl::make_unique<mcedecoder::InterlakenDimmTranslator>());
}

}  // namespace

std::unique_ptr<CpuErrorCountingVisitor>
CreateInterlakenCpuErrorCountingVisitor(
    absl::Time lower_bound,
    std::unique_ptr<mcedecoder::CpuTopologyInterface> cpu_topology) {
  auto mce_decoder = CreateInterlakenMceDecoder(std::move(cpu_topology));
  auto mce_adapter =
      absl::make_unique<MceDecoderAdapter>(std::move(mce_decoder));
  return absl::make_unique<CpuErrorCountingVisitor>(lower_bound,
                                                    std::move(mce_adapter));
}

std::unique_ptr<DimmErrorCountingVisitor>
CreateInterlakenDimmErrorCountingVisitor(
    absl::Time lower_bound,
    std::unique_ptr<mcedecoder::CpuTopologyInterface> cpu_topology) {
  auto mce_decoder = CreateInterlakenMceDecoder(std::move(cpu_topology));
  auto mce_adapter =
      absl::make_unique<MceDecoderAdapter>(std::move(mce_decoder));
  return absl::make_unique<DimmErrorCountingVisitor>(lower_bound,
                                                     std::move(mce_adapter));
}

}  // namespace ecclesia
