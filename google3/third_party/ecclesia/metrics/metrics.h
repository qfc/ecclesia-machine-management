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

#ifndef ECCLESIA_METRICS_METRICS_H_
#define ECCLESIA_METRICS_METRICS_H_

#include <stdint.h>

#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "third_party/opencensus/stats/stats.h"
#include "third_party/opencensus/tags/tag_key.h"

namespace ecclesia::metrics {

// This struct specifies field values to create a OpenCensus View.
struct OpenCensusMetricOptions {
  absl::string_view global_view_name;
  absl::string_view global_measure_name;
  absl::string_view description;
  std::unique_ptr<opencensus::stats::MeasureInt64> measure_int;
  opencensus::tags::TagKey tag_key;
};

// OpenCensusIntCounter represents a component that provides access
// to a resource and records the usage.
class OpenCensusIntCounter {
 public:
  explicit OpenCensusIntCounter(OpenCensusMetricOptions op);

  // Disable copy and assign.
  OpenCensusIntCounter(const OpenCensusIntCounter&) = delete;
  OpenCensusIntCounter& operator=(const OpenCensusIntCounter&) = delete;

  // Records the amount of usage and the id of the object used.
  void IncrementBy(int64_t val, std::string tag_value);

 private:
  const OpenCensusMetricOptions op_;
  std::unique_ptr<opencensus::stats::View> view_;
};

// Registers OpenCensus Exporter for stdout.
// This function can only be called once in the beginning of the binary.
void RegisterExporterForStdout();

}  // namespace ecclesia::metrics

#endif  // ECCLESIA_METRICS_LIB_METRIC_LIB_H_
