licenses(["notice"])

cc_library(
    name = "collector",
    hdrs = ["collector.h"],
    visibility = ["//:mmaster_middle_users"],
    deps = [
        "//google/protobuf:field_mask_cc_proto",
        "@com_google_absl//absl/strings",
        "//mmaster/service:service_cc_proto",
    ],
)

cc_library(
    name = "redfish",
    srcs = ["redfish.cc"],
    hdrs = ["redfish.h"],
    visibility = ["//:mmaster_middle_users"],
    deps = [
        ":collector",
        ":field_mask",
        "//google/protobuf:field_mask_cc_proto",
        "//google/rpc:code_cc_proto",
        "//google/rpc:status_cc_proto",
        "//lib/redfish:result",
        "//mmaster/backends/redfish",
        "//mmaster/service:service_cc_proto",
    ],
)

cc_test(
    name = "redfish_test",
    srcs = ["redfish_test.cc"],
    data = ["//redfish_mockups:indus_hmb_cn_mockup.sar"],
    deps = [
        ":redfish",
        "//net/proto2/contrib/parse_proto:parse_text_proto",
        "//testing/base/public:gunit_main",
        "//lib/redfish:test_mockup",
        "//mmaster/backends/redfish",
        "//mmaster/config:cc_proto",
        "//mmaster/service:service_cc_proto",
    ],
)

cc_library(
    name = "field_mask",
    srcs = ["field_mask.cc"],
    hdrs = ["field_mask.h"],
    visibility = ["//:mmaster_middle_users"],
    deps = [
        "//google/protobuf:field_mask_cc_proto",
        "//net/proto2/public:proto2",
        "//net/proto2/util/public:field_mask_util",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "field_mask_test",
    srcs = ["field_mask_test.cc"],
    deps = [
        ":field_mask",
        "//google/protobuf:field_mask_cc_proto",
        "//testing/base/public:gunit_main",
        "//mmaster/service:service_cc_proto",
    ],
)
