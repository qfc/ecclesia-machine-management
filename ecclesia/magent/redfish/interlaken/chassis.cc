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

#include "ecclesia/magent/redfish/interlaken/chassis.h"

#include <string>
#include <utility>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "ecclesia/magent/redfish/core/json_helper.h"
#include "ecclesia/magent/redfish/core/redfish_keywords.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "ecclesia/magent/sysmodel/x86/fru.h"
#include "ecclesia/magent/sysmodel/x86/sysmodel.h"
#include "json/value.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"

namespace ecclesia {
namespace {

SysmodelFru GetFruInfo(SystemModel *system_model) {
  SysmodelFruReader *mobo_fru_reader =
      system_model->GetFruReader("motherboard");
  if (!mobo_fru_reader) return SysmodelFru({});
  absl::optional<SysmodelFru> fru = mobo_fru_reader->Read();
  if (!fru.has_value()) return SysmodelFru({});
  return std::move(*fru);
}

}  // namespace

void Chassis::Get(ServerRequestInterface *req, const ParamsType &params) {
  Json::Value json;
  json[kOdataType] = "#Chassis.v1_5_0.Chassis";
  json[kOdataId] = std::string(Uri());
  json[kOdataContext] = "/redfish/v1/$metadata#Chassis.Chassis";

  json[kName] = "Interlaken Chassis";
  json[kId] = "chassis";

  SysmodelFru fru = GetFruInfo(system_model_);
  json[kPartNumber] = std::string(fru.GetPartNumber());
  json[kSerialNumber] = std::string(fru.GetSerialNumber());
  json[kManufacturer] = std::string(fru.GetManufacturer());

  auto *assembly = GetJsonObject(&json, "Assembly");
  (*assembly)[kOdataId] = absl::StrCat(Uri(), "/Assembly");

  auto *links = GetJsonObject(&json, "Links");
  auto *computer_systems = GetJsonObject(links, "ComputerSystems");
  (*computer_systems)[kOdataId] = kComputerSystemUri;
  auto *thermal = GetJsonObject(&json, "Thermal");
  (*thermal)[kOdataId] = kThermalUri;

  JSONResponseOK(json, req);
}

}  // namespace ecclesia
