# Description:
# C++ testing utilities for Ecclesia's Redfish interface
licenses(["notice"])

cc_library(
    name = "json_mockup",
    testonly = 1,
    srcs = ["json_mockup.cc"],
    hdrs = ["json_mockup.h"],
    visibility = ["//:redfish_users"],
    deps = [
        "//base",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "//lib/redfish:interface",
        "//third_party/jansson",
    ],
)

cc_test(
    name = "json_mockup_test",
    srcs = ["json_mockup_test.cc"],
    deps = [
        ":json_mockup",
        "//testing/base/public:gunit_main",
        "//lib/redfish:interface",
    ],
)
