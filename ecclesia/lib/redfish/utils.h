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

#ifndef ECCLESIA_LIB_REDFISH_UTILS_H_
#define ECCLESIA_LIB_REDFISH_UTILS_H_

#include <memory>
#include <string>

#include "absl/types/optional.h"
#include "ecclesia/lib/redfish/interface.h"
#include "ecclesia/lib/redfish/property_definitions.h"

namespace libredfish {

// Given a Redfish node that contains a Status, return true if the status is
// “Enabled”; return false otherwise.
//
// An example of such a node is a temperature sensor:
//
// {
//     "@odata.id": "/redfish/v1/Chassis/chassis/Thermal/#/Temperatures/0",
//     "@odata.type": "#Thermal.v1_6_0.Temperature",
//     "RelatedItem": [
//         {
//             "@odata.id": "/redfish/v1/Systems/system/Memory/0"
//         }
//     ],
//     "Status": {
//         "State": "Enabled"
//     },
//     "Name": "dimm0",
//     "ReadingCelsius": 40,
//     "UpperThresholdCritical": 85
// }
inline bool ComponentIsEnabled(libredfish::RedfishObject *node_obj) {
  auto status = node_obj->GetNode(libredfish::kRfPropertyStatus).AsObject();
  if (!status) return false;
  auto state = status->GetNodeValue<libredfish::PropertyState>();
  if (!state) return false;
  return *state == "Enabled";
}

}  // namespace libredfish

#endif
