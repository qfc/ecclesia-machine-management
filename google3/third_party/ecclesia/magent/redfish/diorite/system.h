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

#ifndef ECCLESIA_MAGENT_REDFISH_DIORITE_SYSTEM_H_
#define ECCLESIA_MAGENT_REDFISH_DIORITE_SYSTEM_H_

#include <string>

#include "absl/strings/string_view.h"
#include "magent/redfish/core/json_helper.h"
#include "magent/redfish/core/redfish_keywords.h"
#include "magent/redfish/core/resource.h"
#include "jsoncpp/value.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"

namespace ecclesia {

class ComputerSystem : public Resource {
 public:
  ComputerSystem() : Resource(kComputerSystemUri) {}

 private:
  void Get(ServerRequestInterface *req, const ParamsType &params) override {
    Json::Value json;
    json[kOdataType] = "#ComputerSystem.v1_10_0_.ComputerSystem";
    json[kOdataId] = std::string(Uri());
    json[kOdataContext] = "/redfish/v1/$metadata#ComputerSystem.ComputerSystem";

    // TODO(jaghu) : Some of the data below is fake or not added. This will be
    // filled out in later iteration.
    json[kName] = "Diorite";
    json[kId] = "system";
    json[kSystemType] = "Physical";
    json[kAssetTag] = "free form asset tag";
    json[kManufacturer] = "Manufacturer Name";
    json[kModel] = "Model Name";
    json[kSKU] = "";
    json[kSerialNumber] = "";
    json[kPartNumber] = "";
    json[kUUID] = "";

    auto *status = GetJsonObject(&json, kStatus);
    (*status)[kState] = "Enabled";
    (*status)[kHealth] = "OK";

    json[kIndicatorLED] = "Off";
    json[kPowerState] = "On";

    auto *boot = GetJsonObject(&json, kBoot);
    (*boot)[kBootSourceOverrideEnabled] = "Once";
    (*boot)[kBootSourceOverrideMode] = "UEFI";
    (*boot)[kBootSourceOverrideTarget] = "Pxe";
    (*boot)[kUefiTargetBootSourceOverride] = "";

    json[kBiosVersion] = "P79 v1.00 (09/20/2013)";

    auto *processor_summary = GetJsonObject(&json, kProcessorSummary);
    (*processor_summary)[kCount] = 8;
    (*processor_summary)[kModel] =
        "Multi-Core Intel(R) Xeon(R) processor 7xxx Series";

    auto *memory_summary = GetJsonObject(&json, kMemorySummary);
    (*memory_summary)[kTotalSystemMemoryGiB] = 16;

    auto *links = GetJsonObject(&json, kLinks);
    auto *chassis = GetJsonObject(links, kChassis);
    (*chassis)[kOdataId] = kChassisUri;

    auto *action = GetJsonObject(&json, kActions);
    auto *reset = GetJsonObject(action, kComputerSystemReset);
    (*reset)[kTarget] = "";
    auto *allowable_values = GetJsonArray(reset, kResetTypeAllowableValues);
    allowable_values->append("On");
    allowable_values->append("ForceOff");
    allowable_values->append("ForceRestart");

    JSONResponseOK(json, req);
  }
};
}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_DIORITE_SYSTEM_H_
