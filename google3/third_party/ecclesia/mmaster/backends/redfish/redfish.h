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

#ifndef ECCLESIA_MMASTER_BACKENDS_REDFISH_REDFISH_H_
#define ECCLESIA_MMASTER_BACKENDS_REDFISH_REDFISH_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "lib/redfish/interface.h"
#include "lib/redfish/property.h"
#include "lib/redfish/property_definitions.h"
#include "lib/redfish/raw.h"
#include "lib/redfish/result.h"
#include "lib/redfish/sysmodel.h"
#include "lib/time/clock.h"
#include "mmaster/config/config.proto.h"

namespace ecclesia {

// RedfishBackend provides higher level abstractions on top of a
// RedfishInterface to be used by the Machine Master. These abstractions would
// include querying, enumerating, and acting upon resources wihtout concern
// for the underlying transport layer details.
class RedfishBackend {
 public:
  // Constructs a RedfishBackend with a given RedfishInterface.
  RedfishBackend(AgentConfig config)
      : config_(config),
        redfish_intf_(libredfish::NewRawInterface(absl::StrCat(
            config.redfish().hostname(), ":", config.redfish().port()))),
        property_registry_(libredfish::MakeDefaultPropertyRegistry()),
        sysmodel_(redfish_intf_.get(), property_registry_.get(),
                  Clock::RealClock()) {}

  RedfishBackend(const RedfishBackend &other) = delete;
  RedfishBackend &operator=(const RedfishBackend &other) = delete;

  const std::string &GetAgentName() const { return config_.name(); }
  const std::string &GetOsDomain() const { return config_.os_domain(); }

  // Returns a list of all devpaths reported by the Redfish server.
  std::vector<std::string> GetDevpaths() { return sysmodel_.GetDevpaths(); }
  // Returns a list of all reported plugins and their root devpaths.
  std::vector<libredfish::Result<std::string>> GetPlugins() {
    return sysmodel_.GetPlugins();
  }

  bool DevpathExists(absl::string_view devpath) const {
    return sysmodel_.DevpathExists(devpath);
  }

  // QueryAll invokes result_callback with (devpath, value) tuples of all
  // components in the system model which have the desired property.
  template <typename PropertyDefinitionT>
  void QueryAll(const std::function<
                void(libredfish::Result<typename PropertyDefinitionT::type>)>
                    &result_callback) {
    sysmodel_.QueryAll<PropertyDefinitionT>(result_callback);
  }
  // Query invokes result_callback with (devpath, value) tuples of devpaths
  // providing the desired property.
  template <typename PropertyDefinitionT>
  void Query(absl::Span<const absl::string_view> devpaths,
             const std::function<
                 void(libredfish::Result<typename PropertyDefinitionT::type>)>
                 &result_callback) {
    return sysmodel_.Query<PropertyDefinitionT>(devpaths, result_callback);
  }

  // Helper to check for the presence of a property associated with a devpath.
  // Note that this is a convenience wrapper and it calls Query under the hood.
  // The use of this method is for the use case where the client needs to
  // determine if a given devpath has a associated field.
  // Clients should not be making a Has() call before doing a Query(). In that
  // case the client should just call Query() instead.
  template <typename PropertyDefinitionT>
  bool Has(const absl::string_view devpath) {
    bool result = false;
    this->Query<PropertyDefinitionT>({devpath},
                                     [&result](auto r) { result = true; });
    return result;
  }

 private:
  const AgentConfig config_;
  std::unique_ptr<libredfish::RedfishInterface> redfish_intf_;
  std::unique_ptr<libredfish::PropertyRegistry> property_registry_;
  libredfish::Sysmodel sysmodel_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_BACKENDS_REDFISH_REDFISH_H_
