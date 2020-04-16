licenses(["notice"])

cc_library(
    name = "grpc",
    srcs = ["grpc.cc"],
    hdrs = ["grpc.h"],
    visibility = ["//:mmaster_frontend_users"],
    deps = [
        "//google/protobuf:empty_cc_proto",
        "//net/grpc",
        "//net/grpc:grpc++",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings:str_format",
        "//mmaster/config:cc_proto",
        "//mmaster/middles/aggregator",
        "//mmaster/middles/collector",
        "//mmaster/middles/devpath",
        "//mmaster/service:service_cc_grpc_proto",
        "//mmaster/service:service_cc_proto",
    ],
)
