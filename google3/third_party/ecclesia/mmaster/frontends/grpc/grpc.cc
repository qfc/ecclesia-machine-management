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

#include "mmaster/frontends/grpc/grpc.h"

#include <string>
#include <utility>

#include "google/protobuf/empty.proto.h"
#include "net/grpc/public/include/grpc/grpc_security_constants.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/server_context.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/status.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/sync_stream_impl.h"
#include "net/grpc/public/include/grpcpp/security/server_credentials.h"
#include "net/grpc/public/include/grpcpp/server_builder.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/aggregator/aggregator.h"
#include "mmaster/middles/collector/collector.h"
#include "mmaster/middles/devpath/devpath.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {

MachineMasterFrontend::MachineMasterFrontend(
    DevpathMapper *mapper,
    absl::flat_hash_map<std::string, ResourceCollector *> backends)
    : aggregator_(mapper, std::move(backends)) {}

void MachineMasterFrontend::AddToServerBuilder(
    const FrontendConfig::Grpc &config, ::grpc::ServerBuilder *builder) {
  if (config.has_unix_domain()) {
    builder->AddListeningPort(
        absl::StrFormat("unix:%s", config.unix_domain().path()),
        ::grpc::experimental::LocalServerCredentials(UDS));
  }
  if (config.has_network()) {
    builder->AddListeningPort(
        absl::StrFormat("[::]:%d", config.network().port()),
        ::grpc::experimental::LocalServerCredentials(LOCAL_TCP));
  }
  builder->RegisterService(this);
}

::grpc::Status MachineMasterFrontend::EnumerateOsDomain(
    ::grpc::ServerContext *, const ::google::protobuf::Empty *,
    ::grpc::ServerWriter<EnumerateOsDomainResponse> *writer) {
  return aggregator_.Enumerate(writer);
}

::grpc::Status MachineMasterFrontend::QueryOsDomain(
    ::grpc::ServerContext *,
    ::grpc::ServerReaderWriter<QueryOsDomainResponse, QueryOsDomainRequest>
        *stream) {
  return aggregator_.Query(stream);
}

::grpc::Status MachineMasterFrontend::EnumerateFirmware(
    ::grpc::ServerContext *, const ::google::protobuf::Empty *,
    ::grpc::ServerWriter<EnumerateFirmwareResponse> *writer) {
  return aggregator_.Enumerate(writer);
}

::grpc::Status MachineMasterFrontend::QueryFirmware(
    ::grpc::ServerContext *,
    ::grpc::ServerReaderWriter<QueryFirmwareResponse, QueryFirmwareRequest>
        *stream) {
  return aggregator_.Query(stream);
}

::grpc::Status MachineMasterFrontend::EnumerateStorage(
    ::grpc::ServerContext *, const ::google::protobuf::Empty *,
    ::grpc::ServerWriter<EnumerateStorageResponse> *writer) {
  return aggregator_.Enumerate(writer);
}

::grpc::Status MachineMasterFrontend::QueryStorage(
    ::grpc::ServerContext *,
    ::grpc::ServerReaderWriter<QueryStorageResponse, QueryStorageRequest>
        *stream) {
  return aggregator_.Query(stream);
}

::grpc::Status MachineMasterFrontend::EnumerateAssembly(
    ::grpc::ServerContext *, const ::google::protobuf::Empty *,
    ::grpc::ServerWriter<EnumerateAssemblyResponse> *writer) {
  return aggregator_.Enumerate(writer);
}

::grpc::Status MachineMasterFrontend::QueryAssembly(
    ::grpc::ServerContext *,
    ::grpc::ServerReaderWriter<QueryAssemblyResponse, QueryAssemblyRequest>
        *stream) {
  return aggregator_.Query(stream);
}

::grpc::Status MachineMasterFrontend::EnumerateSensor(
    ::grpc::ServerContext *, const ::google::protobuf::Empty *,
    ::grpc::ServerWriter<EnumerateSensorResponse> *writer) {
  return aggregator_.Enumerate(writer);
}

::grpc::Status MachineMasterFrontend::QuerySensor(
    ::grpc::ServerContext *,
    ::grpc::ServerReaderWriter<QuerySensorResponse, QuerySensorRequest>
        *stream) {
  return aggregator_.Query(stream);
}

}  // namespace ecclesia
