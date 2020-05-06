# Description:
#   Libraries to provide input / output.

licenses(["notice"])

cc_library(
    name = "ioctl",
    srcs = ["ioctl.cc"],
    hdrs = ["ioctl.h"],
    visibility = ["//:library_users"],
)

cc_library(
    name = "msr",
    srcs = ["msr.cc"],
    hdrs = [
        "constants.h",
        "msr.h",
    ],
    visibility = ["//:library_users"],
    deps = [
        "//lib/apifs",
        "//lib/codec:endian",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
    ],
)
