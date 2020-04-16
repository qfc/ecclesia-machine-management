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

#ifndef ECCLESIA_MMASTER_MIDDLES_COLLECTOR_REDFISH_H_
#define ECCLESIA_MMASTER_MIDDLES_COLLECTOR_REDFISH_H_

#include <functional>

#include "google/protobuf/field_mask.proto.h"
#include "mmaster/backends/redfish/redfish.h"
#include "mmaster/middles/collector/collector.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {

class RedfishResourceCollector final : public ResourceCollector {
 public:
  explicit RedfishResourceCollector(RedfishBackend *backend);

  QueryOsDomainResponse Query(
      const OsDomainIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) override;

  void Enumerate(EnumerateFirmwareResponse *response,
                 const std::function<void()> &call_on_write) override;
  QueryFirmwareResponse Query(
      const FirmwareIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) override;

  void Enumerate(EnumerateStorageResponse *response,
                 const std::function<void()> &call_on_write) override;
  QueryStorageResponse Query(
      const StorageIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) override;

  void Enumerate(EnumerateAssemblyResponse *response,
                 const std::function<void()> &call_on_write) override;
  QueryAssemblyResponse Query(
      const AssemblyIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) override;

  void Enumerate(EnumerateSensorResponse *response,
                 const std::function<void()> &call_on_write) override;
  QuerySensorResponse Query(
      const SensorIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) override;

 private:
  RedfishBackend *backend_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MIDDLES_COLLECTOR_REDFISH_H_
