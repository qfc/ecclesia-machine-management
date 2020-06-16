# Description:
#   Libraries to simplify access to API filesystems (e.g. procfs, sysfs).

licenses(["notice"])

cc_library(
    name = "apifs",
    srcs = ["apifs.cc"],
    hdrs = ["apifs.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//ecclesia/lib/cleanup",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)
