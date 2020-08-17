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

#ifndef ECCLESIA_LIB_REDFISH_RAW_H_
#define ECCLESIA_LIB_REDFISH_RAW_H_

#include <memory>
#include <string>

#include "ecclesia/lib/redfish/interface.h"

namespace libredfish {

// Redfish interface connection arguments
struct RedfishInterfaceArgs {
  std::string endpoint;
  std::string username;
  std::string password;
};

// Constructor method for creating a RawInterface.
// Returns nullptr in case the interface failed to be constructed.
std::unique_ptr<RedfishInterface> NewRawInterface(const std::string &endpoint);

// Constructor method for creating a RawInterface with auth session.
// Returns nullptr in case the interface failed to be constructed.
std::unique_ptr<RedfishInterface> NewRawAuthInterface(
    const RedfishInterfaceArgs &connectionArgs);

}  // namespace libredfish

#endif  // ECCLESIA_LIB_REDFISH_RAW_H_
