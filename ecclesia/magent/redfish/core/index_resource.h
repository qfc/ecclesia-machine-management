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
// Unlike the generic resoruce, the URI of this IndexResoruce is normally a
// regex pattern, e.g., "/redfish/v1/Systems/system/Memory/(\\d+)"
class IndexResource : public Resource {
 public:
  // There is no restriction on how to index the collection members. But the
  // index is generally either integer type, e.g.,
  // "/redfish/v1/Systems/system/Memory/0" or string type, e.g.,
  // "/redfish/v1/Chassis/Sleipnir", where the "0" is considered int index type
  // and "Sleipnir" is string index type.
  enum class IndexType { kInt, kString };

  // This constructor assumes int type index. The uri for the resource will be a
  // regex pattern e.g., "/redfish/v1/Systems/system/Memory/(\\d+)"
  explicit IndexResource(const std::string &uri_pattern)
      : IndexResource(uri_pattern, IndexType::kInt) {}

  // This constructor takes in a specific index type. The uri for the resource
  // will be a regex pattern with generic index type. For example:
  // "/redfish/v1/Chassis//(\\w+)" for string-type index.
  IndexResource(const std::string &uri_pattern, IndexType index_type)
      : Resource(uri_pattern), index_type_(index_type) {}

  virtual ~IndexResource() {}

  void RegisterRequestHandler(HTTPServerInterface *server) override {
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
  void RequestHandler(ServerRequestInterface *req) override {
    // The URI of this IndexResoruce is normally a regex pattern. Here we get
    // the aresource index from the request URI. The extracted index will be
    // passed as parameter into the corresponding resource handler.
    RE2 regex(this->Uri());

    if (index_type_ == IndexType::kInt) {
      int int_index;
      if (!RE2::FullMatch(req->uri_path(), regex, &int_index)) {
        req->ReplyWithStatus(HTTPStatusCode::NOT_FOUND);
        return;
      }
      HandleRequestWithIndex(req, int_index);
    } else if (index_type_ == IndexType::kString) {
      std::string str_index;
      if (index_type_ == IndexType::kString &&
          !RE2::FullMatch(req->uri_path(), regex, &str_index)) {
        req->ReplyWithStatus(HTTPStatusCode::NOT_FOUND);
        return;
      }
      HandleRequestWithIndex(req, str_index);
    }
  }

  void HandleRequestWithIndex(ServerRequestInterface *req,
                              const absl::variant<int, std::string> &index) {
    // Pass along the index to the Get() / Post() handlers
    if (req->http_method() == "GET") {
      Get(req, {index});
    } else if (req->http_method() == "POST") {
      Post(req, {index});
    } else {
      req->ReplyWithStatus(HTTPStatusCode::METHOD_NA);
    }
  }

  const IndexType index_type_;
};

}  // namespace ecclesia
#endif  // ECCLESIA_MAGENT_REDFISH_CORE_INDEX_RESOURCE_H_
