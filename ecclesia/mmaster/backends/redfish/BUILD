licenses(["notice"])

cc_library(
    name = "redfish",
    hdrs = ["redfish.h"],
    visibility = ["//:mmaster_backend_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "//lib/redfish:interface",
        "//lib/redfish:property",
        "//lib/redfish:property_definitions",
        "//lib/redfish:raw",
        "//lib/redfish:result",
        "//lib/redfish:sysmodel",
        "//lib/time:clock",
        "//mmaster/config:cc_proto",
    ],
)

cc_test(
    name = "redfish_test",
    srcs = ["redfish_test.cc"],
    data = ["//redfish_mockups:indus_hmb_cn_mockup.sar"],
    deps = [
        ":redfish",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "//lib/redfish:property",
        "//lib/redfish:property_definitions",
        "//lib/redfish:result",
        "//lib/redfish:test_mockup",
        "//mmaster/config:cc_proto",
    ],
)

cc_binary(
    name = "standalone",
    srcs = ["standalone.cc"],
    deps = [
        ":redfish",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "//lib/redfish:property_definitions",
        "//lib/redfish:result",
        "//mmaster/config:cc_proto",
    ],
)

# Interface for testing.
cc_library(
    name = "mockup",
    testonly = 1,
    hdrs = [
        "redfish_mockup.h",
    ],
    visibility = ["//:mmaster_backend_users"],
    deps = [
        ":redfish",
        "@com_google_absl//absl/strings",
        "//lib/redfish:test_mockup",
        "//mmaster/config:cc_proto",
    ],
)
