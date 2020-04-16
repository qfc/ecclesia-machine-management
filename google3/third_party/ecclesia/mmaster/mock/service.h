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

// Implements a mock machine master service where Query responses are provided
// using data collected from a set of text proto files in a directory.

#ifndef ECCLESIA_MMASTER_MOCK_SERVICE_H_
#define ECCLESIA_MMASTER_MOCK_SERVICE_H_

#include <memory>
#include <string>

#include "mmaster/service/service.grpc.pb.h"

namespace ecclesia {

// Create a new mock service given a path to a directory of mocks.
std::unique_ptr<MachineMasterService::Service> MakeMockService(
    const std::string& mocks_dir);

}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MOCK_SERVICE_H_
