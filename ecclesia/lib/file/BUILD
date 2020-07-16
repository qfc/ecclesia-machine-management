licenses(["notice"])

cc_library(
    name = "dir",
    hdrs = ["dir.h"],
    visibility = ["//ecclesia:library_users"],
    deps = ["@com_google_absl//absl/strings"],
)

cc_test(
    name = "dir_test",
    srcs = ["dir_test.cc"],
    deps = [
        ":dir",
        ":test_filesystem",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "path",
    srcs = ["path.cc"],
    hdrs = ["path.h"],
    visibility = ["//ecclesia:library_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "path_test",
    srcs = ["path_test.cc"],
    deps = [
        ":path",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "test_filesystem",
    testonly = True,
    srcs = ["test_filesystem.cc"],
    hdrs = ["test_filesystem.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":dir",
        ":path",
        "//ecclesia/lib/cleanup",
        "//ecclesia/lib/logging",
        "//ecclesia/lib/logging:posix",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "test_filesystem_test",
    srcs = ["test_filesystem_test.cc"],
    deps = [
        ":path",
        ":test_filesystem",
        "//ecclesia/lib/cleanup",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "uds",
    srcs = ["uds.cc"],
    hdrs = ["uds.h"],
    visibility = ["//ecclesia:library_users"],
)

cc_test(
    name = "uds_test",
    srcs = ["uds_test.cc"],
    deps = [
        ":test_filesystem",
        ":uds",
        "@com_google_googletest//:gtest_main",
    ],
)
