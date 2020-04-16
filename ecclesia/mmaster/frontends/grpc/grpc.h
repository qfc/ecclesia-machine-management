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

// This header defines a gRPC based Machine Master frontend backed by one or
// more ResourceCollector-based backends.

#ifndef ECCLESIA_MMASTER_FRONTENDS_GRPC_GRPC_H_
#define ECCLESIA_MMASTER_FRONTENDS_GRPC_GRPC_H_

#include <string>

#include "google/protobuf/empty.proto.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/server_context.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/status.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/sync_stream_impl.h"
#include "net/grpc/public/include/grpcpp/server_builder.h"
#include "absl/container/flat_hash_map.h"
#include "mmaster/config/config.proto.h"
#include "mmaster/middles/aggregator/aggregator.h"
#include "mmaster/middles/collector/collector.h"
#include "mmaster/middles/devpath/devpath.h"
#include "mmaster/service/service.grpc.pb.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {

class MachineMasterFrontend final : public MachineMasterService::Service {
 public:
  MachineMasterFrontend(
      DevpathMapper *mapper,
      absl::flat_hash_map<std::string, ResourceCollector *> backends);

  // Register the frontend with a gRPC server builder using the given config.
  void AddToServerBuilder(const FrontendConfig::Grpc &config,
                          ::grpc::ServerBuilder *builder);

 private:
  ::grpc::Status EnumerateOsDomain(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::grpc::ServerWriter<EnumerateOsDomainResponse> *writer) override;
  ::grpc::Status QueryOsDomain(
      ::grpc::ServerContext *context,
      ::grpc::ServerReaderWriter<QueryOsDomainResponse, QueryOsDomainRequest>
          *stream) override;

  ::grpc::Status EnumerateFirmware(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::grpc::ServerWriter<EnumerateFirmwareResponse> *writer) override;
  ::grpc::Status QueryFirmware(
      ::grpc::ServerContext *context,
      ::grpc::ServerReaderWriter<QueryFirmwareResponse, QueryFirmwareRequest>
          *stream) override;

  ::grpc::Status EnumerateStorage(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::grpc::ServerWriter<EnumerateStorageResponse> *writer) override;
  ::grpc::Status QueryStorage(
      ::grpc::ServerContext *context,
      ::grpc::ServerReaderWriter<QueryStorageResponse, QueryStorageRequest>
          *stream) override;

  ::grpc::Status EnumerateAssembly(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::grpc::ServerWriter<EnumerateAssemblyResponse> *writer) override;
  ::grpc::Status QueryAssembly(
      ::grpc::ServerContext *context,
      ::grpc::ServerReaderWriter<QueryAssemblyResponse, QueryAssemblyRequest>
          *stream) override;

  ::grpc::Status EnumerateSensor(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
      ::grpc::ServerWriter<EnumerateSensorResponse> *writer) override;
  ::grpc::Status QuerySensor(
      ::grpc::ServerContext *context,
      ::grpc::ServerReaderWriter<QuerySensorResponse, QuerySensorRequest>
          *stream) override;

  ResourceAggregator aggregator_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_FRONTENDS_GRPC_GRPC_H_
