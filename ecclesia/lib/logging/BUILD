# Description:
#   Libraries to for supporting the logging of errors and information.

licenses(["notice"])

cc_library(
    name = "logging",
    srcs = ["globals.cc"],
    hdrs = [
        "globals.h",
        "interfaces.h",
        "logging.h",
    ],
    visibility = ["//ecclesia:library_users"],
    deps = [
        "//ecclesia/lib/time:clock",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "posix",
    srcs = ["posix.cc"],
    hdrs = ["posix.h"],
    visibility = ["//ecclesia:library_users"],
    deps = [
        ":logging",
        "@com_google_absl//absl/strings:str_format",
    ],
)
