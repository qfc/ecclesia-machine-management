# Description:
#   Libraries to simplify access to API filesystems (e.g. procfs, sysfs).

licenses(["notice"])

cc_library(
    name = "apifs",
    srcs = ["apifs.cc"],
    hdrs = ["apifs.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//visibility:public"],
    deps = [
        "//ecclesia/lib/cleanup",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "apifs_test",
    size = "small",
    srcs = ["apifs_test.cc"],
    deps = [
        ":apifs",
        "//ecclesia/lib/file:test_filesystem",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)
