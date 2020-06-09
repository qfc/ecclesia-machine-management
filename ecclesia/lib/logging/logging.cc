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

#include "lib/logging/logging.h"

#include <memory>
#include <utility>

#include "absl/memory/memory.h"
#include "lib/time/clock.h"

namespace ecclesia {
namespace {

// Create an instance of the default logger.
std::unique_ptr<LeveledLogger> MakeDefaultLogger() {
  auto logger = absl::make_unique<LeveledLogger>(
      absl::make_unique<NullLogger>(), Clock::RealClock());
  logger->AddLogger(1, absl::make_unique<StderrLogger>());
  return logger;
}

// Function that can both get and set the global logger, depending on whether
// or not the parameter is null (it just does a get) or not (it will set the
// parameter and then return it).
//
// This is used to implement both the Get and Setfunctions in a way that
// provides thread-safe lazy initialization, unlike if we used a shared global
// variable between the two functions. Note that this safety doesn't mean that
// setting the value is threadsafe, just that initialization is.
const LeveledLogger &GetOrSetGlobalLogger(
    std::unique_ptr<LeveledLogger> logger) {
  static LeveledLogger *global_logger = MakeDefaultLogger().release();
  if (logger) {
    auto old_logger = absl::WrapUnique(global_logger);
    global_logger = logger.release();
  }
  return *global_logger;
}

}  // namespace

const LeveledLogger &GetGlobalLogger() { return GetOrSetGlobalLogger(nullptr); }

void SetGlobalLogger(std::unique_ptr<LeveledLogger> logger) {
  GetOrSetGlobalLogger(std::move(logger));
}

}  // namespace ecclesia
