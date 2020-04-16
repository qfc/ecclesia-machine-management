# Description:
# C++ wrapper interfaces for libredfish
licenses(["notice"])

cc_library(
    name = "interface",
    hdrs = ["interface.h"],
    visibility = ["//:redfish_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "property",
    srcs = ["property.cc"],
    hdrs = ["property.h"],
    visibility = ["//:redfish_users"],
    deps = [
        ":interface",
        "//base",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_library(
    name = "property_definitions",
    srcs = ["property_definitions.cc"],
    hdrs = ["property_definitions.h"],
    visibility = ["//:redfish_users"],
    deps = [
        ":property",
        "@com_google_absl//absl/time",
    ],
)

cc_test(
    name = "property_test",
    srcs = ["property_test.cc"],
    deps = [
        ":interface",
        ":property",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/time",
        "//lib/redfish/testing:json_mockup",
    ],
)

cc_library(
    name = "raw",
    srcs = ["raw.cc"],
    hdrs = ["raw.h"],
    visibility = ["//:redfish_users"],
    deps = [
        ":interface",
        "//base",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "//third_party/libredfish",
    ],
)

cc_test(
    name = "raw_test",
    srcs = ["raw_test.cc"],
    data = ["//redfish_mockups:indus_hmb_cn_mockup.sar"],
    deps = [
        ":interface",
        ":raw",
        ":test_mockup",
        "//testing/base/public:gunit_main",
    ],
)

cc_library(
    name = "topology",
    hdrs = ["topology.h"],
    deps = [
        ":property",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "sysmodel",
    srcs = ["sysmodel.cc"],
    hdrs = ["sysmodel.h"],
    visibility = ["//:redfish_users"],
    deps = [
        ":interface",
        ":property",
        ":property_definitions",
        ":result",
        ":topology",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
        "//lib/time:clock",
    ],
)

cc_test(
    name = "sysmodel_test",
    srcs = ["sysmodel_test.cc"],
    deps = [
        ":property",
        ":result",
        ":sysmodel",
        ":topology",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
        "//lib/redfish/testing:json_mockup",
        "//lib/time:clock_fake",
    ],
)

cc_library(
    name = "result",
    hdrs = ["result.h"],
    visibility = [
        "//:mmaster_backend_users",
        "//:redfish_users",
    ],
    deps = ["@com_google_absl//absl/strings"],
)

cc_library(
    name = "test_mockup",
    testonly = 1,
    srcs = ["test_mockup.cc"],
    hdrs = ["test_mockup.h"],
    visibility = [
        "//:mmaster_backend_users",
        "//:redfish_users",
    ],
    deps = [
        ":interface",
        ":raw",
        "//base",
        "//devtools/build/runtime:get_runfiles_dir",
        "//file/base:path",
        "//net/util:ports",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/strings",
        "//util/process:shellprocess",
    ],
)
