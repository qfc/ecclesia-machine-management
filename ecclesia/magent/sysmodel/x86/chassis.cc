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

#include "ecclesia/magent/sysmodel/x86/chassis.h"

#include <string>
#include <vector>

namespace ecclesia {

std::vector<ChassisId> CreateChassis() {
  std::vector<ChassisId> chassis_vec;
  // Hardcode the Indus Chassis for now.
  chassis_vec.push_back(ChassisId::kIndus);
  // Assemblies. Here it expects to dynamically detect and add expenstion
  // Chassis. e.g., Sleipnir.
  return chassis_vec;
}

}  // namespace ecclesia
