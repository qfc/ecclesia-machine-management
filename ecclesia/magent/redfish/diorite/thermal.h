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

#ifndef ECCLESIA_MAGENT_REDFISH_DIORITE_THERMAL_H_
#define ECCLESIA_MAGENT_REDFISH_DIORITE_THERMAL_H_

#include <string>

#include "absl/strings/string_view.h"
#include "magent/redfish/core/json_helper.h"
#include "magent/redfish/core/redfish_keywords.h"
#include "magent/redfish/core/resource.h"
#include "jsoncpp/value.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"

namespace ecclesia {

class Thermal : public Resource {
 public:
  Thermal() : Resource(kThermalUri) {}

 private:
  void Get(ServerRequestInterface *req, const ParamsType &params) override {
    Json::Value json;
    json[kOdataType] = "#Thermal.v1_6_0.Thermal";
    json[kOdataId] = std::string(Uri());
    json[kOdataContext] = "/redfish/v1/$metadata#Thermal.Thermal";
    json[kName] = "Thermal";
    json[kId] = "Thermal";

    // TODO(jaghu) : Some of the data below is fake or not added. This will be
    // filled out in later iteration.
    auto *temperatures = GetJsonObject(&json, kTemperatures);
    (*temperatures)[kOdataId] = kTemperaturesUri;
    (*temperatures)[kReadingCelsius] = 21;
    (*temperatures)[kUpperThresholdNonCritical] = 42;
    (*temperatures)[kUpperThresholdCritical] = 42;
    (*temperatures)[kUpperThresholdFatal] = 42;
    (*temperatures)[kPhysicalContext] = "CPU";

    auto *status = GetJsonObject(temperatures, kStatus);
    (*status)[kState] = "Enabled";
    (*status)[kHealth] = "OK";

    JSONResponseOK(json, req);
  }
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_DIORITE_THERMAL_H_
