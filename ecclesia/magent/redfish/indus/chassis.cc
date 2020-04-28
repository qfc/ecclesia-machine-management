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

#include "magent/redfish/indus/chassis.h"

#include "magent/redfish/core/redfish_keywords.h"
#include "magent/sysmodel/fru.h"
#include "magent/sysmodel/sysmodel.h"

namespace ecclesia {
namespace {

Fru GetFruInfo(SystemModel *system_model) {
  // TODO(dwangsf): Check the FRU ID you want to use and make sure it is
  // referenced here. For now, hardcode it as "motherboard".
  auto mobo_fru = system_model->GetFru("motherboard");
  if (mobo_fru.has_value()) return std::move(*mobo_fru);
  return Fru({});
}

}  // namespace

void Chassis::Get(ServerRequestInterface *req, const ParamsType &params) {
  Json::Value json;
  json[kOdataType] = "#Chassis.v1_5_0.Chassis";
  json[kOdataId] = std::string(Uri());
  json[kOdataContext] = "/redfish/v1/$metadata#Chassis.Chassis";
  // TODO: Get platform name via SysmodelParams
  json[kName] = "Indus Chassis";
  json[kId] = "chassis";

  Fru fru = GetFruInfo(system_model_);
  json[kPartNumber] = std::string(fru.GetPartNumber());
  json[kSerialNumber] = std::string(fru.GetSerialNumber());
  json[kManufacturer] = std::string(fru.GetManufacturer());

  auto *assembly = GetJsonObject(&json, "Assembly");
  (*assembly)[kOdataId] = absl::StrCat(Uri(), "/Assembly");

  auto *links = GetJsonObject(&json, "Links");
  auto *computer_systems = GetJsonObject(links, "ComputerSystems");
  (*computer_systems)[kOdataId] = kComputerSystemUri;
  JSONResponseOK(json, req);
}

}  // namespace ecclesia
