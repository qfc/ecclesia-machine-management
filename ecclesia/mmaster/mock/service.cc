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

#include "mmaster/mock/service.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "google/protobuf/empty.proto.h"  // IWYU pragma: keep
#include "google/rpc/code.proto.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/call_op_set.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/server_context.h"  // IWYU pragma: keep
#include "net/grpc/public/include/grpcpp/impl/codegen/status.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/sync_stream.h"
#include "net/grpc/public/include/grpcpp/impl/codegen/sync_stream_impl.h"  // IWYU pragma: keep
#include "net/proto2/public/text_format.h"
#include "net/proto2/util/public/message_differencer.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "lib/file/dir.h"
#include "mmaster/service/service.grpc.pb.h"  // IWYU pragma: keep
#include "mmaster/service/service.proto.h"  // IWYU pragma: keep

namespace ecclesia {
namespace {

// RAII object for closing a file descriptor.
class FdCloser {
 public:
  explicit FdCloser(int fd) : fd_(fd) {}
  FdCloser(const FdCloser &other) = delete;
  FdCloser &operator=(const FdCloser &other) = delete;
  ~FdCloser() { close(fd_); }

 private:
  int fd_;
};

// RAII object for closing the dirent result from scandir().
class ScandirCloser {
 public:
  explicit ScandirCloser(struct dirent **namelist, int n)
      : namelist_(namelist), n_(n) {}
  ScandirCloser(const ScandirCloser &other) = delete;
  ScandirCloser &operator=(const ScandirCloser &other) = delete;
  ~ScandirCloser() {
    while (n_--) free(namelist_[n_]);
    free(namelist_);
  }

 private:
  struct dirent **namelist_;
  int n_;
};

// Given a directory, return a map of Resource -> List of Filenames where the
// filenames in each list are all names of the form "${Resource}.*.textpb".
// Any files in the directory that do not match this pattern will be ignored.
absl::flat_hash_map<std::string, std::vector<std::string>> FindMockFiles(
    const std::string &mocks_dir) {
  absl::flat_hash_map<std::string, std::vector<std::string>> filenames;
  WithEachFileInDirectory(
      mocks_dir, [mocks_dir, &filenames](absl::string_view filename) {
        std::vector<absl::string_view> parts = absl::StrSplit(filename, '.');
        if (parts.size() == 3 && parts[2] == "textpb") {
          filenames[std::string(parts[0])].push_back(
              absl::StrFormat("%s/%s", mocks_dir, filename));
        }
      });
  return filenames;
}

// Given a list of files for a given type of resource, load them into a list of
// QueryResourceResponse values.
template <typename ResponseType>
std::vector<ResponseType> LoadMockResponses(
    const std::vector<std::string> &filenames) {
  std::vector<ResponseType> response_list;

  for (const std::string &filename : filenames) {
    // Open up the proto file.
    const int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
      std::cerr << "error opening proto file: " << filename;
      continue;
    }
    FdCloser fd_closer(fd);

    // Read in the full contents of the file.
    bool read_error = false;
    std::string file_contents;
    while (true) {
      char buffer[4096];
      const ssize_t n = read(fd, buffer, sizeof(buffer));
      if (n < 0) {
        // Continue on EINTR.
        const auto read_errno = errno;
        if (read_errno == EINTR) continue;
        // Otherwise flag an error and give up.
        read_error = true;
        break;
      } else if (n == 0) {
        break;  // Nothing left to read.
      } else {
        file_contents.append(buffer, n);
      }
    }

    // Close the file and skip this file if an error occurred.
    if (read_error) {
      std::cerr << "error reading proto file: " << filename;
      continue;
    }

    // Parse data into a proto. If that fails skip the file.
    ResponseType response;
    if (!::proto2::TextFormat::ParseFromString(file_contents, &response)) {
      std::cerr << "error parsing text proto from file: " << filename;
      continue;
    }

    // Stuff the proto into the hash map using the ID field.
    response_list.push_back(std::move(response));
  }

  return response_list;
}

// Defines an object that can help handle Enumerate and Query requests for a
// given type of Resource.
//
// This does the bulk of the work that a pair of RPCs would need to do in order
// to response with mock data so that all of actual RPC implementations can be
// minimal stubs that just call into this template. We can't implement the RPCs
// themselves generically since they are parameterized on function names and not
// just types.
template <typename EnumerateResponseType, typename QueryRequestType,
          typename QueryResponseType>
class GenericResourceHandler {
 public:
  explicit GenericResourceHandler(const std::vector<std::string> &filenames)
      : responses_(LoadMockResponses<QueryResponseType>(filenames)) {}

  // Implement the Enumerate operation. This simply serves up the "id" field
  // from every known query response.
  ::grpc::Status Enumerate(
      ::grpc::ServerWriter<EnumerateResponseType> *writer) {
    for (const auto &query_response : responses_) {
      EnumerateResponseType enum_response;
      *enum_response.mutable_id() = query_response.id();
      writer->Write(enum_response, ::grpc::WriteOptions());
    }
    return ::grpc::Status::OK;
  }

  // Implement the Query operation. For each request sent this will take the
  // "id" field from the request and look for a mock query response with the
  // same "id". If it finds one it will send that back; if it does not it will
  // send a response with a "not found" error.
  ::grpc::Status Query(
      ::grpc::ServerReaderWriter<QueryResponseType, QueryRequestType> *stream) {
    QueryRequestType request;
    while (stream->Read(&request)) {
      bool found_response = false;
      // Search through all of the possible responses for a match.
      for (const auto &possible_response : responses_) {
        if (::proto2::util::MessageDifferencer::Equivalent(
                request.id(), possible_response.id())) {
          stream->Write(possible_response, ::grpc::WriteOptions());
          found_response = true;
          break;
        }
      }
      // If no match is found respond with an error instead.
      if (!found_response) {
        QueryResponseType error_response;
        *error_response.mutable_id() = request.id();
        error_response.mutable_status()->set_code(::google::rpc::NOT_FOUND);
        stream->Write(error_response, ::grpc::WriteOptions());
      }
    }
    return ::grpc::Status::OK;
  }

 private:
  const std::vector<QueryResponseType> responses_;
};

// Pull in the definition of a generated MockMachineMaster class.
#include "mmaster/mock/mock_impl.h"

}  // namespace

std::unique_ptr<MachineMasterService::Service> MakeMockService(
    const std::string &mocks_dir) {
  return absl::make_unique<MockMachineMaster>(mocks_dir);
}

}  // namespace ecclesia
