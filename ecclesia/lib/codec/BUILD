# Description:
#   Various libraries for (en)coding and decoding data into different binary
#   formats. Includes features for very specific formats (e.g. encoding and
#   decoding text into 6-bit packed ASCII) as well as more basic things like
#   loading and storing big and little endian integers.

licenses(["notice"])

package(default_visibility = ["//ecclesia:library_users"])

cc_library(
    name = "bits",
    hdrs = ["bits.h"],
    deps = ["@com_google_absl//absl/base:core_headers"],
)

cc_test(
    name = "bits_test",
    size = "small",
    srcs = ["bits_test.cc"],
    deps = [
        ":bits",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "endian",
    hdrs = ["endian.h"],
    visibility = ["//ecclesia:library_users"],
    deps = ["@com_google_absl//absl/numeric:int128"],
)

cc_test(
    name = "endian_test",
    srcs = ["endian_test.cc"],
    deps = [
        ":endian",
        "@com_google_absl//absl/numeric:int128",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "text",
    srcs = ["text.cc"],
    hdrs = ["text.h"],
    deps = [
        ":bits",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "text_test",
    srcs = ["text_test.cc"],
    deps = [
        ":text",
        "@com_google_absl//absl/status",
        "@com_google_googletest//:gtest_main",
    ],
)
