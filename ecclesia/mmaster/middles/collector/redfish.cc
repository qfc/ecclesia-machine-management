// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mmaster/middles/collector/redfish.h"

#include <functional>
#include <string>

#include "google/protobuf/field_mask.proto.h"
#include "google/rpc/code.proto.h"
#include "google/rpc/status.proto.h"
#include "lib/redfish/result.h"
#include "mmaster/backends/redfish/redfish.h"
#include "mmaster/middles/collector/collector.h"
#include "mmaster/middles/collector/field_mask.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {

RedfishResourceCollector::RedfishResourceCollector(RedfishBackend *backend)
    : ResourceCollector(backend->GetAgentName(), backend->GetOsDomain()),
      backend_(backend) {}

QueryOsDomainResponse RedfishResourceCollector::Query(
    const OsDomainIdentifier &id,
    const ::google::protobuf::FieldMask &field_mask) {
  QueryOsDomainResponse response;
  TrimResponseMessage(field_mask, &response);
  response.mutable_status()->set_code(::google::rpc::OK);
  return response;
}

void RedfishResourceCollector::Enumerate(
    EnumerateFirmwareResponse *response,
    const std::function<void()> &call_on_write) {
  // TODO(b/145145628): hook up an implementation that supports FW from Redfish
}
QueryFirmwareResponse RedfishResourceCollector::Query(
    const FirmwareIdentifier &id,
    const ::google::protobuf::FieldMask &field_mask) {
  // TODO(b/145145628): hook up an implementation that supports FW from Redfish
  QueryFirmwareResponse response;
  return response;
}

void RedfishResourceCollector::Enumerate(
    EnumerateStorageResponse *response,
    const std::function<void()> &call_on_write) {
  // TODO(b/143986168): write an implementation that actually enumerates.
}
QueryStorageResponse RedfishResourceCollector::Query(
    const StorageIdentifier &id,
    const ::google::protobuf::FieldMask &field_mask) {
  // TODO(b/143986168): write an implementation that actually enumerates.
  QueryStorageResponse response;
  return response;
}

void RedfishResourceCollector::Enumerate(
    EnumerateAssemblyResponse *response,
    const std::function<void()> &call_on_write) {
  for (const auto &plugin : backend_->GetPlugins()) {
    response->mutable_id()->set_devpath(plugin.devpath);
    call_on_write();
  }
}
QueryAssemblyResponse RedfishResourceCollector::Query(
    const AssemblyIdentifier &id,
    const ::google::protobuf::FieldMask &field_mask) {
  // TODO(dchanman): Make this more efficient. Right now, this is O(n^2) to
  // enumerate and then query plugin names. Implement something in the backend
  // to do a O(1) lookup for a given devpath.
  QueryAssemblyResponse response;
  for (const auto &plugin : backend_->GetPlugins()) {
    if (plugin.devpath == id.devpath()) {
      response.set_name(plugin.value);
      // TODO(dchanman): Also query for part and serial numbers
      TrimResponseMessage(field_mask, &response);
      response.mutable_status()->set_code(::google::rpc::OK);
      return response;
    }
  }

  response.mutable_status()->set_code(::google::rpc::NOT_FOUND);
  return response;
}

void RedfishResourceCollector::Enumerate(
    EnumerateSensorResponse *response,
    const std::function<void()> &call_on_write) {
  // TODO(b/147144287): support sensors from Redfish
}
QuerySensorResponse RedfishResourceCollector::Query(
    const SensorIdentifier &id,
    const ::google::protobuf::FieldMask &field_mask) {
  // TODO(b/147144287): support sensors from Redfish
  QuerySensorResponse response;
  return response;
}

}  // namespace ecclesia
