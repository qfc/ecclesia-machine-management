licenses(["notice"])

cc_library(
    name = "devpath",
    srcs = ["devpath.cc"],
    hdrs = ["devpath.h"],
    visibility = ["//ecclesia:library_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "transform",
    srcs = ["transform.cc"],
    hdrs = ["transform.h"],
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
