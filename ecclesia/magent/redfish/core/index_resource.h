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

#ifndef ECCLESIA_MAGENT_REDFISH_CORE_INDEX_RESOURCE_H_
#define ECCLESIA_MAGENT_REDFISH_CORE_INDEX_RESOURCE_H_

#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/variant.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"
#include "tensorflow_serving/util/net_http/server/public/response_code_enum.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"
#include "re2/re2.h"

namespace ecclesia {
// When a redfish resource is a part of a collection, and the uri contains an
// index to the resource, prefer to derive from this class.
// Example: Memory resource
class IndexResource : public Resource {
 public:
  // The uri for the resource will be a regex pattern
  // For example: "/redfish/v1/Systems/system/Memory/(\\d+)"
  explicit IndexResource(const std::string &uri_pattern)
      : Resource(uri_pattern) {}

  virtual ~IndexResource() {}

  virtual void RegisterRequestHandler(HTTPServerInterface *server) override {
    server->RegisterRequestDispatcher(
        [this](ServerRequestInterface *http_request)
            -> tensorflow::serving::net_http::RequestHandler {
          RE2 regex(this->Uri());
          if (RE2::FullMatch(http_request->uri_path(), regex)) {
            return [this](ServerRequestInterface *req) {
              return this->RequestHandler(req);
            };
          } else {
            return nullptr;
          }
        },
        RequestHandlerOptions());
  }

 protected:
  // Helper method to validate the resource index from the request URI
  // To be called from the Get/Post methods
  bool ValidateResourceIndex(const ParamsType &params, int num_resources) {
    if (params.size() != 1 || !absl::holds_alternative<int>(params[0]) ||
        std::get<int>(params[0]) >= num_resources ||
        std::get<int>(params[0]) < 0) {
      return false;
    }
    return true;
  }

 private:
  virtual void RequestHandler(ServerRequestInterface *req) override {
    // Get the resource index from the request uri
    RE2 regex(this->Uri());
    int index;
    if (!RE2::FullMatch(req->uri_path(), regex, &index)) {
      req->ReplyWithStatus(HTTPStatusCode::NOT_FOUND);
      return;
    }
    // Pass along the index to the Get() / Post() handlers
    if (req->http_method() == "GET") {
      Get(req, {index});
    } else if (req->http_method() == "POST") {
      Post(req, {index});
    } else {
      req->ReplyWithStatus(HTTPStatusCode::METHOD_NA);
    }
  }
};

}  // namespace ecclesia
#endif  // ECCLESIA_MAGENT_REDFISH_CORE_INDEX_RESOURCE_H_
