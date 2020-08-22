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

#ifndef ECCLESIA_MAGENT_REDFISH_INTERLAKEN_CHASSIS_H_
#define ECCLESIA_MAGENT_REDFISH_INTERLAKEN_CHASSIS_H_

#include <string>

#include "absl/strings/string_view.h"
#include "ecclesia/magent/redfish/core/json_helper.h"
#include "ecclesia/magent/redfish/core/redfish_keywords.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "ecclesia/magent/sysmodel/x86/sysmodel.h"
#include "json/value.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"

namespace ecclesia {

class ChassisCollection : public Resource {
 public:
  ChassisCollection() : Resource(kChassisCollectionUri) {}

 private:
  void Get(ServerRequestInterface *req, const ParamsType &params) override {
    Json::Value json;
    json[kOdataType] = "#ChassisCollection.ChassisCollection";
    json[kOdataId] = std::string(Uri());
    json[kOdataContext] =
        "/redfish/v1/"
        "$metadata#ChassisCollection.ChassisCollection";
    json[kName] = "Interlaken";
    json[kMembersCount] = 1;
    auto *json_members = GetJsonArray(&json, kMembers);
    AppendCollectionMember(json_members, kChassisUri);
    JSONResponseOK(json, req);
  }
};

class Chassis : public Resource {
 public:
  Chassis(SystemModel *system_model)
      : Resource(kChassisUri), system_model_(system_model) {}

 private:
  void Get(ServerRequestInterface *req, const ParamsType &params) override;

  SystemModel *const system_model_;
};

}  // namespace ecclesia
#endif  // ECCLESIA_MAGENT_REDFISH_INTERLAKEN_CHASSIS_H_
