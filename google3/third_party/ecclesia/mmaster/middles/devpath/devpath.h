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

// Defines a generic interface for performing mappings between management agent
// devpaths and machine devpaths. These mappings go in both directions: it can
// be used to take devpaths reported from a management domain and turn them into
// full machine devpaths, and it can also be used to take a machine devpath and
// transform it back into the original domain devpath.

#ifndef ECCLESIA_MMASTER_MIDDLES_DEVPATH_DEVPATH_H_
#define ECCLESIA_MMASTER_MIDDLES_DEVPATH_DEVPATH_H_

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace ecclesia {
class DevpathMapper {
 public:
  DevpathMapper() = default;
  virtual ~DevpathMapper() = default;

  // Given domain-specific devpath, transform it into a machine devpath. Returns
  // nullopt if the devpath is not valid within the given domain.
  virtual absl::optional<std::string> DomainDevpathToMachineDevpath(
      absl::string_view domain, absl::string_view devpath) const = 0;

  // Given a domain and a machine devpath, transform it into a domain-specific
  // devpath. Returns nullopt if the devpath is not valid within the given
  // domain.
  virtual absl::optional<std::string> MachineDevpathToDomainDevpath(
      absl::string_view domain, absl::string_view devpath) const = 0;

  // Given a machine devpath return a list of all domains which it can be mapped
  // into. This list may be empty, or it may contain multiple domains.
  virtual std::vector<absl::string_view> MachineDevpathToDomains(
      absl::string_view devpath) const = 0;
};
}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MIDDLES_DEVPATH_DEVPATH_H_
