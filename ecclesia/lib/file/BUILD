licenses(["notice"])

cc_library(
    name = "dir",
    hdrs = ["dir.h"],
    visibility = ["//ecclesia:library_users"],
    deps = ["@com_google_absl//absl/strings"],
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

cc_library(
    name = "test_filesystem",
    testonly = True,
    srcs = ["test_filesystem.cc"],
    hdrs = ["test_filesystem.h"],
    visibility = ["//ecclesia:library_users"],
    deps = [
        ":dir",
        ":path",
        "//ecclesia/lib/cleanup",
        "//ecclesia/lib/logging",
        "//ecclesia/lib/logging:posix",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "uds",
    srcs = ["uds.cc"],
    hdrs = ["uds.h"],
    visibility = ["//ecclesia:library_users"],
)
