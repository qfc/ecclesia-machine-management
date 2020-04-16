# Description:
#   Libraries to simplify access to API filesystems (e.g. procfs, sysfs).

licenses(["notice"])

cc_library(
    name = "apifs",
    srcs = ["apifs.cc"],
    hdrs = ["apifs.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "//lib/cleanup",
    ],
)

cc_test(
    name = "apifs_test",
    size = "small",
    srcs = ["apifs_test.cc"],
    deps = [
        ":apifs",
        "//platforms/util/testing:fake_filesystem",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)
