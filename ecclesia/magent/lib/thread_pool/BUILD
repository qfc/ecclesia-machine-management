licenses(["notice"])

cc_library(
    name = "thread_pool",
    hdrs = ["thread_pool.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
    ],
)
