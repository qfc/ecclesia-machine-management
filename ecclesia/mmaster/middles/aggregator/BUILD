licenses(["notice"])

cc_library(
    name = "aggregator",
    srcs = ["aggregator.cc"],
    hdrs = ["aggregator.h"],
    visibility = ["//:mmaster_middle_users"],
    deps = [
        "//google/rpc:code_cc_proto",
        "//net/grpc:grpc++",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
        "//mmaster/middles/collector",
        "//mmaster/middles/devpath",
        "//mmaster/service:service_cc_grpc_proto",
        "//mmaster/service:service_cc_proto",
    ],
)
