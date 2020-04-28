# Description:
#   Libraries to provide input / output.

licenses(["notice"])

cc_library(
    name = "msr",
    srcs = ["msr.cc"],
    hdrs = [
        "constants.h",
        "msr.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
        "//lib/apifs",
        "//lib/codec:endian",
    ],
)

cc_test(
    name = "msr_test",
    size = "small",
    srcs = ["msr_test.cc"],
    deps = [
        ":msr",
        "//platforms/util/testing:fake_filesystem",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)
