load("//net/goa/build_defs:generate_rpc_client_legacy.bzl", "goa_generate_rpc_client_legacy")
load("//net/grpc:cc_grpc_library.bzl", "cc_grpc_library")
load("//tools/build_defs/proto/cpp:cc_proto_library.bzl", "cc_proto_library")
load("//build_defs:constants.bzl", "constant", "constant_lib")

licenses(["notice"])

cc_library(
    name = "identifiers",
    hdrs = ["identifiers.h"],
    visibility = ["//:mmaster_frontend_users"],
    deps = ["@com_google_absl//absl/strings"],
)

constant_lib(
    name = "ports",
    cc_namespace = "ecclesia",
    constants = [
        # Defines the standard default port numbers and paths that the mmaster uses.
        constant(
            cc_name = "kMmasterDefaultNetworkPort",
            go_name = "MmasterDefaultNetworkPort",
            py_name = "MMASTER_DEFAULT_NETWORK_PORT",
            value = 3996,
        ),
        constant(
            cc_name = "kMmasterDefaultUdsPath",
            go_name = "MmasterDefaultUdsPath",
            py_name = "MMASTER_DEFAULT_UDS_PATH",
            value = "/var/run/mmaster/rpc.socket",
        ),
    ],
    visibility = ["//:mmaster_users"],
)

proto_library(
    name = "service_proto",
    srcs = ["service.proto"],
    has_services = 1,
    cc_api_version = 2,
    deps = [
        "//google/protobuf:duration",
        "//google/protobuf:empty",
        "//google/protobuf:field_mask",
        "//google/protobuf:timestamp",
        "//google/rpc:status",
    ],
)

go_proto_library(
    name = "service_go_proto",
    visibility = ["//:mmaster_users"],
    deps = [":service_proto"],
)

goa_generate_rpc_client_legacy(
    name = "mmasterclient",
    proto_rule = ":service_go_proto",
    server_spec_flag = "mmaster_addr",
    service_name = "MachineMasterService",
    visibility = ["//:mmaster_users"],
)

cc_proto_library(
    name = "service_cc_proto",
    visibility = ["//:mmaster_users"],
    deps = [":service_proto"],
)

cc_grpc_library(
    name = "service_cc_grpc_proto",
    srcs = [":service_proto"],
    visibility = ["//:mmaster_users"],
    deps = [":service_cc_proto"],
)

cc_test(
    name = "service_test",
    srcs = ["service_test.cc"],
    deps = [
        ":identifiers",
        ":service_cc_proto",
        "//base",
        "//net/proto2/public:proto2",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)
