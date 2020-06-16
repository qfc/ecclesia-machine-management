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

#ifndef ECCLESIA_MAGENT_REDFISH_INDUS_PROCESSOR_H_
#define ECCLESIA_MAGENT_REDFISH_INDUS_PROCESSOR_H_

#include <string>
#include <type_traits>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "ecclesia/magent/redfish/core/index_resource.h"
#include "ecclesia/magent/redfish/core/json_helper.h"
#include "ecclesia/magent/redfish/core/redfish_keywords.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "ecclesia/magent/sysmodel/x86/cpu.h"
#include "ecclesia/magent/sysmodel/x86/sysmodel.h"
#include "json/value.h"
#include "tensorflow_serving/util/net_http/server/public/response_code_enum.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"

namespace ecclesia {

class Processor : public IndexResource {
 public:
  explicit Processor(SystemModel *system_model)
      : IndexResource(kProcessorUriPattern), system_model_(system_model) {}

 private:
  void Get(ServerRequestInterface *req, const ParamsType &params) override {
    // Expect to be passed in the cpu index
    if (!ValidateResourceIndex(params, system_model_->NumCpus())) {
      req->ReplyWithStatus(HTTPStatusCode::NOT_FOUND);
      return;
    }
    // Fill in the json response
    auto cpu = system_model_->GetCpu(std::get<int>(params[0]));
    auto cpu_info = cpu->GetCpuInfo();
    Json::Value json;
    json[kOdataType] = "#Processor.v1_6_0.Processor";
    json[kOdataId] = std::string(req->uri_path());
    json[kOdataContext] = "/redfish/v1/$metadata#Processor.Processor";
    json[kName] = cpu_info.name;

    if (cpu_info.enabled) {
      json[kMaxSpeedMHz] = cpu_info.max_speed_mhz;
      json[kSerialNumber] = cpu_info.serial_number;
      json[kPartNumber] = cpu_info.part_number;
      json[kTotalCores] = cpu_info.total_cores;
      json[kTotalEnabledCores] = cpu_info.enabled_cores;
      json[kTotalThreads] = cpu_info.total_threads;
      auto *assembly = GetJsonObject(&json, kAssembly);
      (*assembly)[kOdataId] = absl::StrCat(req->uri_path(), "/", kAssembly);
      if (cpu_info.cpu_signature) {
        json[kManufacturer] = cpu_info.cpu_signature->vendor;
        auto *processor_id = GetJsonObject(&json, kProcessorId);
        (*processor_id)[kEffectiveFamily] = cpu_info.cpu_signature->family;
        (*processor_id)[kEffectiveModel] = cpu_info.cpu_signature->model;
        (*processor_id)[kStep] = cpu_info.cpu_signature->stepping;
        (*processor_id)[kVendorId] = cpu_info.cpu_signature->vendor;
      }
    }

    auto *metrics = GetJsonObject(&json, kMetrics);
    (*metrics)[kOdataId] =
        absl::StrCat(req->uri_path(), "/", kProcessorMetrics);

    auto *status = GetJsonObject(&json, kStatus);
    (*status)[kState] = cpu_info.enabled ? "Enabled" : "Absent";

    JSONResponseOK(json, req);
  }

  SystemModel *const system_model_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_INDUS_PROCESSOR_H_
