licenses(["notice"])

cc_library(
    name = "devpath",
    srcs = ["devpath.cc"],
    hdrs = ["devpath.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:library_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "devpath_test",
    srcs = ["devpath_test.cc"],
    deps = [
        ":devpath",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "transform",
    srcs = ["transform.cc"],
    hdrs = ["transform.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:mmaster_middle_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:field_mask_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

proto_library(
    name = "transform_test_proto",
    testonly = True,
    srcs = ["transform_test.proto"],
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
        "//ecclesia/lib/protobuf:parse",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)
