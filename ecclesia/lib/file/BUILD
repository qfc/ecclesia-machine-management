licenses(["notice"])

cc_library(
    name = "uds",
    srcs = ["uds.cc"],
    hdrs = ["uds.h"],
    visibility = ["//:library_users"],
)

cc_library(
    name = "dir",
    hdrs = ["dir.h"],
    visibility = ["//:library_users"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
)
