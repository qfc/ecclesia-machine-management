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

#include "ecclesia/magent/redfish/core/assembly.h"

#include <filesystem>
#include <fstream>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "ecclesia/magent/redfish/core/redfish_keywords.h"
#include "ecclesia/magent/redfish/core/resource.h"
#include "json/json.h"
#include "json/value.h"
#include "tensorflow_serving/util/net_http/server/public/httpserver_interface.h"
#include "tensorflow_serving/util/net_http/server/public/server_request_interface.h"
#include "re2/re2.h"

namespace ecclesia {

namespace {

// Parse a text JSON file and return a json object corresponding to the root
Json::Value ReadJsonFile(const std::string &file_path) {
  std::string file_contents;
  std::string line;
  std::ifstream file(file_path);
  if (!file.is_open()) return Json::Value();
  while (getline(file, line)) {
    file_contents.append(line);
  }
  file.close();
  Json::Reader reader;
  Json::Value value;
  if (reader.parse(file_contents, value)) {
    return value;
  }
  return Json::Value();
}

// Given a path to a directory containing json files that represent assembly
// resources, this function loads up the assemblies into dictionary,
absl::flat_hash_map<std::string, Json::Value> GetAssemblies(
    absl::string_view dir_path) {
  absl::flat_hash_map<std::string, Json::Value> assemblies;
  for (auto &p : std::filesystem::directory_iterator(std::string(dir_path))) {
    std::string file_path = std::string(p.path());
    if (absl::EndsWith(file_path, ".json")) {
      Json::Value root = ReadJsonFile(file_path);
      if (root[kOdataId].isString()) {
        assemblies[root[kOdataId].asString()] = root;
      }
    }
  }
  return assemblies;
}

}  // namespace

Assembly::Assembly(absl::string_view assemblies_dir)
    : Resource(kAssemblyUriPattern),
      assemblies_(GetAssemblies(assemblies_dir)) {}

void Assembly::RegisterRequestHandler(HTTPServerInterface *server) {
  server->RegisterRequestDispatcher(
      [this](ServerRequestInterface *http_request)
          -> tensorflow::serving::net_http::RequestHandler {
        RE2 regex(this->Uri());
        if (RE2::FullMatch(http_request->uri_path(), regex)) {
          return [this](ServerRequestInterface *req) {
            this->RequestHandler(req);
          };
        } else {
          return nullptr;
        }
      },
      RequestHandlerOptions());
}

}  // namespace ecclesia
