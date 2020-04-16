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

#ifndef ECCLESIA_MMASTER_SERVICE_IDENTIFIERS_H_
#define ECCLESIA_MMASTER_SERVICE_IDENTIFIERS_H_

#include "absl/strings/string_view.h"

namespace ecclesia {

// An array with the names of all of the resource types supported by the machine
// master service interface. The type names match the names used in the messages
// and RPC names in the service definition.
inline constexpr absl::string_view kServiceResourceTypes[] = {
    "Assembly", "Firmware", "OsDomain", "PowerDomain", "Sensor", "Storage",
};

// An array with the names of all mutate verbs and their associated service
// resource supported by the machine master service interface. The names listed
// here must match the Mutate.* RPC names in the service definition.
struct ResourceVerb {
  absl::string_view resource;
  absl::string_view verb;
};
inline constexpr ResourceVerb kResourceVerbs[] = {
    {"PowerDomain", "Reset"},
};
}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_SERVICE_IDENTIFIERS_H_
