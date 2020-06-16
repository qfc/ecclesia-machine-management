# Description:
#   Libraries to provide input / output.

licenses(["notice"])

cc_library(
    name = "ioctl",
    srcs = ["ioctl.cc"],
    hdrs = ["ioctl.h"],
    visibility = ["//ecclesia:library_users"],
)

cc_library(
    name = "msr",
    srcs = ["msr.cc"],
    hdrs = [
        "constants.h",
        "msr.h",
    ],
    visibility = ["//ecclesia:library_users"],
    deps = [
        "//ecclesia/lib/apifs",
        "//ecclesia/lib/codec:endian",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
    ],
)
