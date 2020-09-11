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

#ifndef ECCLESIA_MAGENT_REDFISH_CORE_METADATA_H_
#define ECCLESIA_MAGENT_REDFISH_CORE_METADATA_H_

#include <fstream>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/variant.h"
#include "ecclesia/lib/logging/logging.h"
#include "ecclesia/magent/redfish/core/redfish_keywords.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"
#include "tensorflow_serving/util/net_http/server/public/response_code_enum.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"
#include "re2/re2.h"

namespace ecclesia {

using tensorflow::serving::net_http::HTTPServerInterface;
using tensorflow::serving::net_http::HTTPStatusCode;
using tensorflow::serving::net_http::RequestHandlerOptions;
using tensorflow::serving::net_http::ServerRequestInterface;

// (DSP0266, Section 8.4.1): "The OData metadata describes top-level service
// resources and resource types."
class ODataMetadata {
 public:
  // Initialize the csdl metadata at construction
  explicit ODataMetadata(const std::string & odata_metadata_path) {
    std::ifstream metadata_file;
    std::string line;
    metadata_file.open(odata_metadata_path.c_str());
    if (metadata_file.good()) {
      while (std::getline(metadata_file, line)) {
        metadata_text_ = absl::StrCat(metadata_text_, line);
      }
      metadata_file.close();
    } else {
      ecclesia::ErrorLog() << "Failed to open OData metadata file.";
    }
  }

  virtual ~ODataMetadata() {}

  virtual void RegisterRequestHandler(HTTPServerInterface *server) {
    RequestHandlerOptions handler_options;
    server->RegisterRequestHandler(
        kODataMetadataUri,
        [this](ServerRequestInterface *req) {
          return this->RequestHandler(req);
        },
        handler_options);
  }

 private:
  virtual void RequestHandler(ServerRequestInterface *req) {
    if (req->http_method() == "GET") {
      tensorflow::serving::net_http::SetContentType(req,
                                                    "text/xml; charset=UTF-8");
      req->WriteResponseString(metadata_text_);
      req->ReplyWithStatus(HTTPStatusCode::OK);
    } else {
      req->ReplyWithStatus(HTTPStatusCode::METHOD_NA);
    }
  }

  std::string metadata_text_;
};

// Factory function to create a OData Metadata instance with request handler.
inline std::unique_ptr<ODataMetadata> CreateMetadata(
    HTTPServerInterface *server, const std::string & odata_metadata_path) {
  auto metadata = absl::make_unique<ODataMetadata>(odata_metadata_path);
  metadata->RegisterRequestHandler(server);
  return metadata;
}

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_REDFISH_CORE_METADATA_H_
