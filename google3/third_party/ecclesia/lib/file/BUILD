licenses(["notice"])

cc_library(
    name = "uds",
    srcs = ["uds.cc"],
    hdrs = ["uds.h"],
    visibility = ["//:library_users"],
    deps = ["//base"],
)

cc_test(
    name = "uds_test",
    srcs = ["uds_test.cc"],
    deps = [
        ":uds",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/flags:flag",
    ],
)

cc_library(
    name = "dir",
    hdrs = ["dir.h"],
    visibility = ["//:library_users"],
    deps = [
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "dir_test",
    srcs = ["dir_test.cc"],
    deps = [
        ":dir",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
    ],
)
