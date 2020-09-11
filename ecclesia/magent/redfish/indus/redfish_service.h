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
#include "ecclesia/magent/redfish/core/odata_metadata.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "ecclesia/magent/sysmodel/x86/sysmodel.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"

namespace ecclesia {

// The top level class which encapsulates all the redfish resources exported by
// the management agent.
class IndusRedfishService {
 public:
  explicit IndusRedfishService(HTTPServerInterface *server,
                               SystemModel *system_model,
                               absl::string_view assemblies_dir,
                               const std::string &odata_metadata_path);

  IndusRedfishService(const IndusRedfishService &) = delete;
  IndusRedfishService &operator=(const IndusRedfishService &) = delete;

 private:
  std::vector<std::unique_ptr<Resource>> resources_;
  std::unique_ptr<ODataMetadata> metadata_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_INDUS_REDFISH_SERVICE_H_
