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

#ifndef ECCLESIA_MAGENT_REDFISH_CORE_COLLECTION_RESOURCE_H_
#define ECCLESIA_MAGENT_REDFISH_CORE_COLLECTION_RESOURCE_H_

#include <string>

#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/types/variant.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"
#include "tensorflow_serving/util/net_http/server/public/response_code_enum.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"

namespace ecclesia {
// When a redfish resource is a part of a collection, prefer to derive from
// this class.
// Examples: Memory resource, Assemblies
class ServiceRootResource : public Resource {
 public:
  explicit ServiceRootResource(const std::string &uri) : Resource(uri) {}

  virtual ~ServiceRootResource() {}

  // Register a request handler to route requests corresponding to uri_.
  // This treats the URI with a trailing forward slash as equivalent to a
  // request without a trailing forward slash.
  void RegisterRequestHandler(HTTPServerInterface *server) override {
    server->RegisterRequestDispatcher(
        [this](ServerRequestInterface *http_request)
            -> tensorflow::serving::net_http::RequestHandler {
          if (http_request->uri_path() == this->Uri()) {
            return [this](ServerRequestInterface *req) {
              return this->RequestHandler(req);
            };
          } else if (http_request->uri_path() ==
                     absl::StripSuffix(this->Uri(), "/")) {
            return [this](ServerRequestInterface *req) {
              return this->RedirectHandler(req);
            };
          } else {
            return nullptr;
          }
        },
        RequestHandlerOptions());
  }

 private:
  void RedirectHandler(ServerRequestInterface *req) {
    req->OverwriteResponseHeader("Location", this->Uri());
    req->ReplyWithStatus(HTTPStatusCode::MOVED_PERM);
  }
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_CORE_COLLECTION_RESOURCE_H_
