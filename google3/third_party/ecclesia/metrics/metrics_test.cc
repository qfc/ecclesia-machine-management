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

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "third_party/opencensus/stats/stats.h"
#include "third_party/opencensus/tags/tag_key.h"

using ::opencensus::stats::MeasureInt64;
using ::opencensus::tags::TagKey;

namespace ecclesia::metrics {
namespace {

using ::opencensus::stats::ViewData;
using ::opencensus::stats::ViewDescriptor;

constexpr absl::string_view kInternalCommFailureName =
    "ecclesia/mmaster/internal_communication_failure";
constexpr absl::string_view kInternalCommFailureDesc =
    "Total number of internal communication failures for machine master";

MeasureInt64 InternalCommFailureMeasure() {
  static const MeasureInt64 m = MeasureInt64::Register(
      kInternalCommFailureName, kInternalCommFailureDesc, "1");
  return m;
}

TagKey ErrorTypeKey() {
  static const auto key = opencensus::tags::TagKey::Register("error_type");
  return key;
}

// A mock exporter that assigns exported data to the provided pointer.
//
// The atomic<bool> is used to synchronize access to the given output object.
// The ExportViewData will only write to the output point if the bool is set to
// true, and after it does such a write it will set it back to false. The caller
// can then know that the output is set and can be safely accessed by waiting
// for the atomic<bool> to be false.
class MockExporter : public ::opencensus::stats::StatsExporter::Handler {
 public:
  MockExporter(std::vector<std::pair<ViewDescriptor, ViewData>> *output,
               std::atomic<bool> *write_output)
      : output_(output), write_output_(write_output) {}

  void ExportViewData(
      const std::vector<std::pair<ViewDescriptor, ViewData>> &data) override {
    if (!write_output_) return;
    // Looping because ViewData is (intentionally) copy-constructable but not
    // copy_assignable.
    for (const auto &datum : data) {
      output_->emplace_back(datum.first, datum.second);
    }
    *write_output_ = false;
  }

 private:
  std::vector<std::pair<ViewDescriptor, ViewData>> *output_;
  std::atomic<bool> *write_output_;
};

}  // namespace

class MetricsTest : public ::testing::Test {
 protected:
  MetricsTest() {
    OpenCensusMetricOptions op{
        kInternalCommFailureName,
        kInternalCommFailureName,
        kInternalCommFailureDesc,
        std::make_unique<MeasureInt64>(InternalCommFailureMeasure()),
        {ErrorTypeKey()}};

    counter1_ = absl::make_unique<OpenCensusIntCounter>(std::move(op));
  }

  std::unique_ptr<OpenCensusIntCounter> counter1_;
};

TEST_F(MetricsTest, IncrmentCounter) {
  // Register an exporter
  std::vector<std::pair<ViewDescriptor, ViewData>> exported_data;
  std::atomic<bool> write_to_exported_data = false;
  opencensus::stats::StatsExporter::RegisterPushHandler(
      absl::make_unique<MockExporter>(&exported_data, &write_to_exported_data));

  counter1_->IncrementBy(1, "agent_unreachable");
  write_to_exported_data = true;

  while (write_to_exported_data) {
    absl::SleepFor(absl::Milliseconds(50));
  }

  const std::pair<std::vector<std::string>, int64_t>
      kExpectedInternalCommFailure({{"agent_unreachable"}, 1});

  std::vector<std::pair<std::vector<std::string>, int64_t>> collected_metrics;
  for (const auto &datum : exported_data) {
    for (const auto &data : datum.second.int_data()) {
      collected_metrics.push_back(data);
    }
  }

  EXPECT_THAT(collected_metrics,
              ::testing::UnorderedElementsAre(kExpectedInternalCommFailure));
}

}  // namespace ecclesia::metrics
