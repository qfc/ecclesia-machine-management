# Description:
# C++ testing utilities for Ecclesia's Redfish interface
licenses(["notice"])

cc_library(
    name = "json_mockup",
    testonly = 1,
    srcs = ["json_mockup.cc"],
    hdrs = ["json_mockup.h"],
    visibility = ["//ecclesia:redfish_users"],
    deps = [
        "//ecclesia/lib/logging",
        "//ecclesia/lib/redfish:interface",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
        "@jansson",
    ],
)

cc_test(
    name = "json_mockup_test",
    srcs = ["json_mockup_test.cc"],
    deps = [
        ":json_mockup",
        "//ecclesia/lib/redfish:interface",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)
