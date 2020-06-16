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

#ifndef ECCLESIA_MAGENT_REDFISH_INDUS_REDFISH_SERVICE_H_
#define ECCLESIA_MAGENT_REDFISH_INDUS_REDFISH_SERVICE_H_

#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "ecclesia/magent/redfish/core/assembly.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "ecclesia/magent/redfish/indus/chassis.h"
#include "ecclesia/magent/redfish/indus/memory.h"
#include "ecclesia/magent/redfish/indus/memory_collection.h"
#include "ecclesia/magent/redfish/indus/memory_metrics.h"
#include "ecclesia/magent/redfish/indus/processor.h"
#include "ecclesia/magent/redfish/indus/processor_collection.h"
#include "ecclesia/magent/redfish/indus/processor_metrics.h"
#include "ecclesia/magent/redfish/indus/root.h"
#include "ecclesia/magent/redfish/indus/service_root.h"
#include "ecclesia/magent/redfish/indus/system.h"
#include "ecclesia/magent/redfish/indus/systems.h"
#include "ecclesia/magent/sysmodel/x86/sysmodel.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"

namespace ecclesia {

// The top level class which encapsulates all the redfish resources exported by
// the management agent.
class IndusRedfishService {
 public:
  explicit IndusRedfishService(HTTPServerInterface *server,
                               SystemModel *system_model,
                               absl::string_view assemblies_dir) {
    resources_.push_back(CreateResource<Root>(server));
    resources_.push_back(CreateResource<ServiceRoot>(server));
    resources_.push_back(CreateResource<ComputerSystemCollection>(server));
    resources_.push_back(CreateResource<ComputerSystem>(server));
    resources_.push_back(CreateResource<ChassisCollection>(server));
    resources_.push_back(CreateResource<Chassis>(server, system_model));
    resources_.push_back(
        CreateResource<MemoryCollection>(server, system_model));
    resources_.push_back(CreateResource<Memory>(server, system_model));
    resources_.push_back(CreateResource<Assembly>(server, assemblies_dir));
    resources_.push_back(CreateResource<MemoryMetrics>(server, system_model));
    resources_.push_back(
        CreateResource<ProcessorCollection>(server, system_model));

    resources_.push_back(CreateResource<Processor>(server, system_model));
    resources_.push_back(
        CreateResource<ProcessorMetrics>(server, system_model));
  }

  IndusRedfishService(const IndusRedfishService &) = delete;
  IndusRedfishService &operator=(const IndusRedfishService &) = delete;

 private:
  std::vector<std::unique_ptr<Resource>> resources_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_INDUS_REDFISH_SERVICE_H_
