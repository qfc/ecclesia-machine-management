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

// Implements the aggregation and routing of Ecclesia resource oriented
// operations (enumeration, querying, and mutation) across multiple agents using
// the generic ResourceCollector interface.

#ifndef ECCLESIA_MMASTER_MIDDLES_AGGREGATOR_AGGERGATOR_H_
#define ECCLESIA_MMASTER_MIDDLES_AGGREGATOR_AGGERGATOR_H_

#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "google/rpc/code.proto.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/call_op_set.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/status.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/sync_stream.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/sync_stream_impl.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/string_view.h"
#include "mmaster/middles/collector/collector.h"
#include "mmaster/middles/devpath/devpath.h"
#include "mmaster/service/service.grpc.pb.h"  // IWYU pragma: keep
#include "mmaster/service/service.proto.h"

namespace ecclesia {

class ResourceAggregator {
 public:
  ResourceAggregator(
      DevpathMapper *mapper,
      absl::flat_hash_map<std::string, ResourceCollector *> backends);

  // Implement the mechanics of an Enumerate RPC.
  template <typename ResponseType>
  ::grpc::Status Enumerate(::grpc::ServerWriter<ResponseType> *writer) {
    ResponseType response;
    // Cycle through each backend, querying them for for these resources.
    for (const auto &entry : backends_) {
      const std::string &domain = entry.first;
      ResourceCollector *collector = entry.second;

      // For each resource reported by the collector transform the domain
      // devpaths into machine devpaths and then (if that transformating works)
      // stream it out to the client immediately.
      collector->Enumerate(&response, [&]() {
        bool transform_worked = true;
        for (std::string *devpath : GetDevpathPointers(response.mutable_id())) {
          absl::optional<std::string> maybe_new_devpath =
              mapper_->DomainDevpathToMachineDevpath(domain, *devpath);
          if (maybe_new_devpath) {
            *devpath = *maybe_new_devpath;
          } else {
            transform_worked = false;
            break;
          }
        }
        if (transform_worked) {
          writer->Write(response, ::grpc::WriteOptions());
        } else {
          // TODO(b/142060834): we need to log this failure somewhere
        }
      });
    }
    return ::grpc::Status::OK;
  }

  template <>
  ::grpc::Status Enumerate<EnumerateOsDomainResponse>(
      ::grpc::ServerWriter<EnumerateOsDomainResponse> *writer) {
    absl::flat_hash_set<absl::string_view> os_domains;
    for (const auto &domain_to_collector : backends_) {
      os_domains.insert(domain_to_collector.second->GetOsDomain());
    }
    for (const auto &os_domain : os_domains) {
      EnumerateOsDomainResponse response;
      response.mutable_id()->set_name(os_domain);
      writer->Write(response, ::grpc::WriteOptions());
    }
    return ::grpc::Status::OK;
  }

  // Implement the mechanics of a Query RPC.
  template <typename ResponseType, typename RequestType>
  ::grpc::Status Query(
      ::grpc::ServerReaderWriter<ResponseType, RequestType> *stream) {
    RequestType request;
    while (stream->Read(&request)) {
      // Extract the identifier from the request and query each of the domains
      // that may be aware of it. If any one of them succeeds, return that data.
      // If all of them fail (or there are no domains) return a NOT_FOUND error.
      auto id = request.id();
      ResponseType response;
      bool found_response = false;
      for (absl::string_view domain : GetDomains(&id)) {
        // Get the backend for that domain, or move on if it doesn't exist.
        auto backend_iter = backends_.find(domain);
        if (backend_iter == backends_.end()) continue;
        ResourceCollector *collector = backend_iter->second;
        // Construct a domain-specific ID message. If that isn't possible then
        // move on to the next domain.
        auto domain_id = id;
        bool transform_worked = true;
        for (std::string *devpath : GetDevpathPointers(&domain_id)) {
          absl::optional<std::string> maybe_new_devpath =
              mapper_->MachineDevpathToDomainDevpath(domain, *devpath);
          if (maybe_new_devpath) {
            *devpath = *maybe_new_devpath;
          } else {
            transform_worked = false;
            break;
          }
        }
        if (!transform_worked) continue;
        // Try querying the backend. If it gave an OK response then stop and use
        // that. Otherwise, keep trying more backends.
        ResponseType candidate_response =
            collector->Query(domain_id, request.field_mask());
        if (candidate_response.status().code() == ::google::rpc::OK) {
          response = std::move(candidate_response);
          found_response = true;
          break;
        }
      }
      // We're done querying backends. Either we have a response that we can
      // write out or we need to set NOT_FOUND in the status field. In both
      // cases we also need to fill the response id field with the original
      // from the source request.
      *response.mutable_id() = std::move(id);
      if (!found_response) {
        response.mutable_status()->set_code(::google::rpc::NOT_FOUND);
      }
      stream->Write(response, ::grpc::WriteOptions());
    }
    return ::grpc::Status::OK;
  }

 private:
  // Given a Resource ID message, provide a function that will return a vector
  // of string pointers to fields that represent devpaths.
  //
  // The output of this function will then be used to transform devpaths between
  // domain-specific and machine-global paths. An overload of this function must
  // be implemented for each resource type.
  static std::vector<std::string *> GetDevpathPointers(OsDomainIdentifier *id);
  static std::vector<std::string *> GetDevpathPointers(FirmwareIdentifier *id);
  static std::vector<std::string *> GetDevpathPointers(StorageIdentifier *id);
  static std::vector<std::string *> GetDevpathPointers(AssemblyIdentifier *id);
  static std::vector<std::string *> GetDevpathPointers(SensorIdentifier *id);

  static std::vector<std::string *> GetOsDomainPointers(OsDomainIdentifier *id);

  std::vector<absl::string_view> FindDomainsFromDevpaths(
      const std::vector<std::string *> &devpaths) const;

  // Define type_traits defining how management domains are to be selected for
  // all identifiers.
  template <typename T>
  struct domain_differentiation {
    static constexpr bool by_devpath = false;
    static constexpr bool by_os_domain = false;
  };
  template <>
  struct domain_differentiation<OsDomainIdentifier> {
    static constexpr bool by_devpath = false;
    static constexpr bool by_os_domain = true;
  };
  template <>
  struct domain_differentiation<FirmwareIdentifier> {
    static constexpr bool by_devpath = true;
    static constexpr bool by_os_domain = false;
  };
  template <>
  struct domain_differentiation<StorageIdentifier> {
    static constexpr bool by_devpath = true;
    static constexpr bool by_os_domain = false;
  };
  template <>
  struct domain_differentiation<AssemblyIdentifier> {
    static constexpr bool by_devpath = true;
    static constexpr bool by_os_domain = false;
  };
  template <>
  struct domain_differentiation<SensorIdentifier> {
    static constexpr bool by_devpath = true;
    static constexpr bool by_os_domain = false;
  };

  // The following GetDomains implementations specialize depending on the type
  // traits declared for the various identifiers above.
  // GetDomains takes in the identifier as an input, parses its fields, and
  // returns a vector of management domain names which are responsible for
  // handling the RPC.

  // Specialization for types which differentiate domains by devpath only.
  template <typename T,
            std::enable_if_t<(domain_differentiation<T>::by_devpath &&
                              !domain_differentiation<T>::by_os_domain),
                             int> = 0>
  std::vector<absl::string_view> GetDomains(T *id) {
    return FindDomainsFromDevpaths(GetDevpathPointers(id));
  }

  // Specialization for types which differentiate domains by OsDomain only.
  template <typename T,
            std::enable_if_t<(!domain_differentiation<T>::by_devpath &&
                              domain_differentiation<T>::by_os_domain),
                             int> = 0>
  std::vector<absl::string_view> GetDomains(T *id) {
    std::vector<absl::string_view> domains;
    for (const auto &domain_to_collector : backends_) {
      if (domain_to_collector.second->GetOsDomain() == id->name()) {
        domains.push_back(domain_to_collector.first);
      }
    }
    return domains;
  }

  DevpathMapper *mapper_;
  absl::flat_hash_map<std::string, ResourceCollector *> backends_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MIDDLES_AGGREGATOR_AGGERGATOR_H_
