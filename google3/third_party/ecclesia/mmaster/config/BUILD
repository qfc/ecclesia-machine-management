load("//tools/build_defs/proto/cpp:cc_proto_library.bzl", "cc_proto_library")

licenses(["notice"])

proto_library(
    name = "proto",
    srcs = ["config.proto"],
    cc_api_version = 2,
    visibility = ["//:mmaster_config_users"],
)

cc_proto_library(
    name = "cc_proto",
    visibility = ["//:mmaster_config_users"],
    deps = [":proto"],
)

go_proto_library(
    name = "go_proto",
    visibility = ["//:mmaster_config_users"],
    deps = [":proto"],
)

py_proto_library(
    name = "config_py_pb2",
    api_version = 2,
    visibility = ["//:mmaster_config_users"],
    deps = [":proto"],
)
