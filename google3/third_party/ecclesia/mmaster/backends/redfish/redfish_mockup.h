#ifndef ECCLESIA_MMASTER_BACKENDS_REDFISH_REDFISH_MOCKUP_H_
#define ECCLESIA_MMASTER_BACKENDS_REDFISH_REDFISH_MOCKUP_H_

#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "lib/redfish/test_mockup.h"
#include "mmaster/backends/redfish/redfish.h"
#include "mmaster/config/config.proto.h"

namespace ecclesia {

// Class which facilitates Redfish backend testing by spinning up a
// TestingMockupServer and constructing a backend which connects to it.
//
// Only read-only support is provided at the moment.
class MockupRedfishServer {
 public:
  // Spin up a MockupRedfishServer using the provided .sar google3 path.
  // The created backend will use backend_name in its AgentConfig.
  MockupRedfishServer(absl::string_view backend_name,
                      absl::string_view mockup_sar_google3_path)
      : mockup_server_(mockup_sar_google3_path),
        backend_(CreateConfig(backend_name, mockup_server_.GetConfig().hostname,
                              mockup_server_.GetConfig().port)) {}

  RedfishBackend *GetBackend() { return &backend_; }

 private:
  static AgentConfig CreateConfig(absl::string_view backend_name,
                                  absl::string_view hostname, int port) {
    AgentConfig config;
    config.set_name(backend_name);
    config.set_os_domain(absl::StrCat(backend_name, "_os_domain"));
    auto *redfish = config.mutable_redfish();
    redfish->set_hostname(hostname);
    redfish->set_port(port);
    return config;
  }
  libredfish::TestingMockupServer mockup_server_;
  RedfishBackend backend_;
};

}  //  namespace ecclesia

#endif  // ECCLESIA_MMASTER_BACKENDS_REDFISH_REDFISH_MOCKUP_H_
