load("//tools/build_defs/proto/cpp:cc_proto_library.bzl", "cc_proto_library")

licenses(["notice"])

cc_library(
    name = "devpath",
    srcs = ["devpath.cc"],
    hdrs = ["devpath.h"],
    visibility = ["//:library_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "devpath_test",
    srcs = ["devpath_test.cc"],
    deps = [
        ":devpath",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "transform",
    srcs = ["transform.cc"],
    hdrs = ["transform.h"],
    visibility = ["//:mmaster_middle_users"],
    deps = [
        "//google/protobuf:field_mask_cc_proto",
        "//net/proto2/public:proto2",
        "//net/proto2/util/public:field_mask_util",
        "@com_google_absl//absl/strings",
    ],
)

proto_library(
    name = "transform_test_proto",
    testonly = True,
    srcs = ["transform_test.proto"],
    cc_api_version = 2,
)

cc_proto_library(
    name = "transform_test_cc_proto",
    testonly = True,
    deps = [":transform_test_proto"],
)

cc_test(
    name = "transform_test",
    srcs = ["transform_test.cc"],
    deps = [
        ":transform",
        ":transform_test_cc_proto",
        "//net/proto2/contrib/parse_proto:parse_text_proto",
        "//platforms/gsys/proto:gsys_cc_proto",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/strings",
    ],
)
