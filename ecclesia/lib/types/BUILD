licenses(["notice"])

cc_library(
    name = "bytes",
    hdrs = ["bytes.h"],
    visibility = ["//ecclesia:library_users"],
)

cc_library(
    name = "fixed_range_int",
    hdrs = ["fixed_range_int.h"],
    visibility = ["//ecclesia:library_users"],
    deps = ["@com_google_absl//absl/types:optional"],
)

cc_test(
    name = "fixed_range_int_test",
    srcs = ["fixed_range_int_test.cc"],
    deps = [
        ":fixed_range_int",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)
