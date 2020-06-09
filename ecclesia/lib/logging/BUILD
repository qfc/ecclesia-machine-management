# Description:
#   Libraries to provide informational logging.

licenses(["notice"])

cc_library(
    name = "logging",
    srcs = [
        "location.h",
        "logging.cc",
    ],
    hdrs = ["logging.h"],
    deps = [
        "//lib/time:clock",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
    ],
)
