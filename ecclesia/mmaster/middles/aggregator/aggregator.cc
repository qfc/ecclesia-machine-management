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

#include "mmaster/middles/aggregator/aggregator.h"

#include <stddef.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "mmaster/middles/collector/collector.h"
#include "mmaster/middles/devpath/devpath.h"
#include "mmaster/service/service.proto.h"

namespace ecclesia {

ResourceAggregator::ResourceAggregator(
    DevpathMapper *mapper,
    absl::flat_hash_map<std::string, ResourceCollector *> backends)
    : mapper_(mapper), backends_(std::move(backends)) {}

std::vector<std::string *> ResourceAggregator::GetDevpathPointers(
    OsDomainIdentifier *id) {
  return {};
}

std::vector<std::string *> ResourceAggregator::GetDevpathPointers(
    FirmwareIdentifier *id) {
  return {id->mutable_devpath()};
}

std::vector<std::string *> ResourceAggregator::GetDevpathPointers(
    StorageIdentifier *id) {
  return {id->mutable_devpath()};
}

std::vector<std::string *> ResourceAggregator::GetDevpathPointers(
    AssemblyIdentifier *id) {
  return {id->mutable_devpath()};
}

std::vector<std::string *> ResourceAggregator::GetDevpathPointers(
    SensorIdentifier *id) {
  return {id->mutable_devpath()};
}

std::vector<std::string *> ResourceAggregator::GetOsDomainPointers(
    OsDomainIdentifier *id) {
  return {id->mutable_name()};
}

std::vector<absl::string_view> ResourceAggregator::FindDomainsFromDevpaths(
    const std::vector<std::string *> &devpaths) const {
  using DomainSet = std::vector<absl::string_view>;
  DomainSet domains;

  if (devpaths.empty()) return domains;  // No devpaths, no domains.

  // Start off with the set of domains being the domains the first devpath
  // could be from. Then intersect it with the sets of possible domains for all
  // the other devpaths.
  domains = mapper_->MachineDevpathToDomains(*devpaths[0]);
  std::sort(domains.begin(), domains.end());
  for (size_t i = 1; i < devpaths.size(); ++i) {
    DomainSet candidate_domains = std::move(domains);
    DomainSet new_candidate_domains =
        mapper_->MachineDevpathToDomains(*devpaths[i]);
    std::sort(new_candidate_domains.begin(), new_candidate_domains.end());
    absl::c_set_intersection(candidate_domains, new_candidate_domains,
                             std::back_inserter(domains));
  }

  return domains;
}

}  // namespace ecclesia
