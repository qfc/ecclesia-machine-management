// Utility binary used to generate the MockMachineMaster class defined
// internally in service.cc. It writes a class definition to standard output.
//
// Given a set of arbitrary Query*Response protos that module implements a
// generic handler for the corresponding Enumerate and Query RPCs. However,
// because we need to hook declarations up to this handler for every resource we
// define, there's a boilerplate that needs to be updated every time we add a
// new type of resource. Instead of maintaining that by hand, we instead use
// this tool to generate the code for all of the declarations.
//
// This implementation is tightly coupled to the internal code in service.cc. It
// is not intended to produce any kind of generic, reusable output.

#include <iostream>

#include "absl/strings/string_view.h"
#include "absl/strings/substitute.h"
#include "mmaster/service/identifiers.h"

namespace ecclesia {
namespace {

int RealMain(int argc, char *argv[]) {
  // Print out the class declaration and the constructor declaration.
  std::cout
      << R"(class MockMachineMaster final : public MachineMasterService::Service {
 public:
  explicit MockMachineMaster(const std::string &mocks_dir)
      : mock_file_map_(FindMockFiles(mocks_dir)))";

  // For every resource, print out code to initialize the handler member.
  for (absl::string_view resource : kServiceResourceTypes) {
    std::cout << absl::Substitute(
        ",\n        handler_for_$0_(mock_file_map_[\"$0\"])", resource);
  }

  // Close out the constructor and declare the mock_file_map_ member.
  std::cout << R"( {}

 private:
  absl::flat_hash_map<std::string, std::vector<std::string>> mock_file_map_;
)";

  // For each resource define a handler member and both Enumerate and Query
  // implementations that call into the handler.
  for (absl::string_view resource : kServiceResourceTypes) {
    std::cout << absl::Substitute(R"(
  GenericResourceHandler<Enumerate$0Response,
                         Query$0Request, Query$0Response>
      handler_for_$0_;
  ::grpc::Status Enumerate$0(
      ::grpc::ServerContext *context, const ::google::protobuf::Empty *,
      ::grpc::ServerWriter<Enumerate$0Response> *writer) override {
    return handler_for_$0_.Enumerate(writer);
  }
  ::grpc::Status Query$0(
      ::grpc::ServerContext *context,
      ::grpc::ServerReaderWriter<Query$0Response,
                                 Query$0Request> *stream) override {
    return handler_for_$0_.Query(stream);
  }
)",
                                  resource);
  }

  // Close out the class definition.
  std::cout << "};" << std::endl;

  return 0;
}

}  // namespace
}  // namespace ecclesia

int main(int argc, char *argv[]) { return ecclesia::RealMain(argc, argv); }
