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

#ifndef ECCLESIA_MAGENT_REDFISH_INDUS_THERMAL_H_
#define ECCLESIA_MAGENT_REDFISH_INDUS_THERMAL_H_

#include <string>
#include <utility>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "ecclesia/magent/redfish/core/json_helper.h"
#include "ecclesia/magent/redfish/core/redfish_keywords.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "ecclesia/magent/sysmodel/x86/dimm.h"
#include "ecclesia/magent/sysmodel/x86/sysmodel.h"
#include "ecclesia/magent/sysmodel/x86/thermal.h"
#include "json/value.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"

namespace ecclesia {

class Thermal : public Resource {
 public:
  explicit Thermal(SystemModel *system_model)
      : Resource(kThermalUri), system_model_(system_model) {}

 private:
  void Get(ServerRequestInterface *req, const ParamsType &params) override {
    Json::Value json;
    AddStaticFields(&json);
    int num_sensors = system_model_->NumDimmThermalSensors();
    json[kMembersCount] = num_sensors;
    auto *members = GetJsonArray(&json, kTemperatures);

    // CPU thermal is not listed here, because (at least some Intel) CPU only
    // reports thermal margin. Those are listed in ProcessorMetrics.
    for (int i = 0; i < num_sensors; i++) {
      PciThermalSensor *sensor = system_model_->GetDimmThermalSensor(i);

      Json::Value thermal;
      thermal[kOdataId] = absl::StrCat(kThermalUri, "#/Temperatures/", i);
      thermal[kOdataType] = "#Thermal.v1_6_0.Temperature";
      thermal[kName] = std::string(sensor->Name());
      {
        auto temp = sensor->Read();
        if (temp) {
          thermal[kReadingCelsius] = std::move(*temp);
        }
      }
      thermal[kUpperThresholdCritical] = sensor->UpperThresholdCritical();

      Json::Value dimm;
      dimm[kOdataId] = absl::StrCat(kMemoryCollectionUri, "/", i);
      GetJsonArray(&thermal, kRelatedItem)->append(dimm);

      Json::Value status;
      if (system_model_->GetDimm(i)->GetDimmInfo().present) {
        status[kState] = kEnabled;
      } else {
        status[kState] = kAbsent;
      }

      thermal[kStatus] = status;

      members->append(thermal);
    }

    JSONResponseOK(json, req);
  }

  void AddStaticFields(Json::Value *json) {
    (*json)[kOdataType] = "#Thermal.v1_6_0.Thermal";
    (*json)[kOdataId] = std::string(Uri());
    (*json)[kOdataContext] = "/redfish/v1/$metadata#Thermal.Thermal";
    (*json)[kName] = "Thermal";
  }

  SystemModel *const system_model_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_INDUS_THERMAL_H_
