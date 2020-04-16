// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "metrics/metrics.h"

#include <stdint.h>

#include <memory>
#include <string>
#include <utility>

#include "third_party/opencensus/exporters/stats/stdout/stdout_exporter.h"
#include "third_party/opencensus/stats/stats.h"
#include "third_party/opencensus/tags/tag_key.h"

using ::opencensus::stats::MeasureInt64;
using ::opencensus::stats::View;
using ::opencensus::stats::ViewDescriptor;
using ::opencensus::tags::TagKey;

namespace ecclesia::metrics {
namespace {

// The resource owner publicly registers the tag keys used in their recording
// calls so that it is accessible to views.
TagKey ErrorTypeKey() {
  static const auto key = opencensus::tags::TagKey::Register("error_type");
  return key;
}

}  // namespace

OpenCensusIntCounter::OpenCensusIntCounter(OpenCensusMetricOptions op)
    : op_(std::move(op)) {
  ViewDescriptor view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(op_.global_view_name)
          .set_description(op_.description)
          .set_measure(op_.global_measure_name)
          .set_aggregation(opencensus::stats::Aggregation::Sum())
          .add_column(op_.tag_key);

  view_ = std::make_unique<View>(view_descriptor);
  view_descriptor.RegisterForExport();
}

void OpenCensusIntCounter::IncrementBy(int64_t val, std::string tag_value) {
  opencensus::stats::Record({{*op_.measure_int, val}},
                            {{ErrorTypeKey(), tag_value}});
}

void RegisterExporterForStdout() {
  opencensus::exporters::stats::StdoutExporter::Register();
}

}  // namespace ecclesia::metrics
