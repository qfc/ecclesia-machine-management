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

// Defines a generic interface for collecting information about Ecclesia
// resource information from an arbitrary backend.
//
// The provides some abstraction from the exact structure of information on the
// backend as well as providing some generic implementations of common patterns
// of caching and translation.

#ifndef ECCLESIA_MMASTER_MIDDLES_COLLECTOR_COLLECTOR_H_
#define ECCLESIA_MMASTER_MIDDLES_COLLECTOR_COLLECTOR_H_

#include <functional>
#include <string>
#include <utility>

#include "google/protobuf/field_mask.proto.h"
#include "absl/strings/string_view.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {

// This collector basically implements the standard Ecclesia API verbs for each
// type of resource, with the expectation that the collector is running against
// a specific backend.
//
// The verbs are implemented by using a standard name and overloading on the
// message types instead of using different names for different resources. This
// allows them to be used in generic code templated on resource message types.
class ResourceCollector {
 public:
  ResourceCollector(std::string management_domain, std::string os_domain)
      : management_domain_(std::move(management_domain)),
        os_domain_(std::move(os_domain)) {}
  virtual ~ResourceCollector() = default;

  absl::string_view GetManagementDomain() const { return management_domain_; }
  absl::string_view GetOsDomain() const { return os_domain_; }

  // Enumeration functions. These functions all work the same way: they will
  // write a response into the given pointer and then call the supplied
  // callback after each write.
  //
  // Implementations MUST NOT assume that the ID message is in a particular
  // state when it is first called, or after each call_on_write invocation.
  // In particular, it should not assume that the ID message will either be
  // cleared or left unmodified. Since most enumeration implementations will be
  // overwriting all of the fields in the message anyway, this should usually
  // not be an issue.
  //
  // The types are defined using this rather roundabout approach because:
  //   - they are virtual and so can't be templated on a function parameter
  //   - overloading on different std::function types can fail in a lot of
  //     confusing and subtle ways since the types you pass in don't always
  //     get converted in the way you expect
  // Taking a pointer to the ID message makes the overload resolution simple
  // and straightforward.
  //
  // A much simpler signature would be to just return a vector or some other
  // collection, but supplying the values to the caller one at a time allows
  // data to be streamed out as it comes in.
  virtual void Enumerate(EnumerateFirmwareResponse *response,
                         const std::function<void()> &call_on_write) = 0;
  virtual void Enumerate(EnumerateStorageResponse *response,
                         const std::function<void()> &call_on_write) = 0;
  virtual void Enumerate(EnumerateAssemblyResponse *response,
                         const std::function<void()> &call_on_write) = 0;
  virtual void Enumerate(EnumerateSensorResponse *response,
                         const std::function<void()> &call_on_write) = 0;

  // Query functions. These functions always expect a FirmwareIdentifier and
  // a field mask and will return an response proto.
  //
  // The response proto will have its status field populated, as well as any
  // fields specified by the field mask. The response id field WILL NOT be
  // populated by the function.
  virtual QueryOsDomainResponse Query(
      const OsDomainIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) = 0;
  virtual QueryFirmwareResponse Query(
      const FirmwareIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) = 0;
  virtual QueryStorageResponse Query(
      const StorageIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) = 0;
  virtual QueryAssemblyResponse Query(
      const AssemblyIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) = 0;
  virtual QuerySensorResponse Query(
      const SensorIdentifier &id,
      const ::google::protobuf::FieldMask &field_mask) = 0;

 private:
  std::string management_domain_;
  std::string os_domain_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MIDDLES_COLLECTOR_COLLECTOR_H_
