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

#ifndef ECCLESIA_MAGENT_REDFISH_DIORITE_REDFISH_SERVICE_H_
#define ECCLESIA_MAGENT_REDFISH_DIORITE_REDFISH_SERVICE_H_

#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "magent/redfish/core/assembly.h"
#include "magent/redfish/core/resource.h"
#include "magent/redfish/diorite/chassis.h"
#include "magent/redfish/diorite/power.h"
#include "magent/redfish/diorite/root.h"
#include "magent/redfish/diorite/service_root.h"
#include "magent/redfish/diorite/system.h"
#include "magent/redfish/diorite/systems.h"
#include "magent/redfish/diorite/thermal.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"

namespace ecclesia {

// The top level class which encapsulates all the redfish resources exported by
// the management agent.
class DioriteRedfishService {
 public:
  explicit DioriteRedfishService(HTTPServerInterface *server,
                                 absl::string_view assemblies_dir) {
    resources_.push_back(CreateResource<Root>(server));
    resources_.push_back(CreateResource<ServiceRoot>(server));
    resources_.push_back(CreateResource<ComputerSystemCollection>(server));
    resources_.push_back(CreateResource<ComputerSystem>(server));
    resources_.push_back(CreateResource<ChassisCollection>(server));
    resources_.push_back(CreateResource<Chassis>(server));
    resources_.push_back(CreateResource<Power>(server));
    resources_.push_back(CreateResource<Thermal>(server));
    resources_.push_back(CreateResource<Assembly>(server, assemblies_dir));
  }

  DioriteRedfishService(const DioriteRedfishService &) = delete;
  DioriteRedfishService &operator=(const DioriteRedfishService &) = delete;

 private:
  std::vector<std::unique_ptr<Resource>> resources_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_DIORITE_REDFISH_SERVICE_H_
