licenses(["notice"])

cc_binary(
    name = "generate",
    srcs = ["generate.cc"],
    deps = [
        "@com_google_absl//absl/strings",
        "//mmaster/service:identifiers",
    ],
)

genrule(
    name = "generate_mock_impl_h",
    outs = ["mock_impl.h"],
    cmd = "$(location :generate) > $@",
    tools = [":generate"],
)

cc_library(
    name = "mock_impl_h",
    # This "header" is not a standalone library. Instead it is a generated code
    # fragment intended for inclusion into service.cc.
    textual_hdrs = [":generate_mock_impl_h"],
)

cc_library(
    name = "service",
    srcs = ["service.cc"],
    hdrs = ["service.h"],
    deps = [
        ":mock_impl_h",
        "//google/protobuf:empty_cc_proto",
        "//google/rpc:code_cc_proto",
        "//net/grpc:grpc++",
        "//net/proto2/public:proto2",
        "//net/proto2/util/public:differencer",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "//lib/file:dir",
        "//mmaster/service:service_cc_grpc_proto",
        "//mmaster/service:service_cc_proto",
    ],
)

cc_binary(
    name = "mock",
    srcs = ["main.cc"],
    visibility = ["//:mmaster_mock_users"],
    deps = [
        ":service",
        "//net/grpc",
        "//net/grpc:grpc++",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings:str_format",
        "//mmaster/service:service_cc_grpc_proto",
    ],
)
