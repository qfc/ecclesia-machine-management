# Description:
#   Various libraries for (en)coding and decoding data into different binary
#   formats. Includes features for very specific formats (e.g. encoding and
#   decoding text into 6-bit packed ASCII) as well as more basic things like
#   loading and storing big and little endian integers.

licenses(["notice"])

package(default_visibility = ["//:library_users"])

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
        "//testing/base/public:gunit_main",
    ],
)

cc_library(
    name = "endian",
    hdrs = ["endian.h"],
    visibility = ["//:library_users"],
    deps = ["@com_google_absl//absl/numeric:int128"],
)

cc_test(
    name = "endian_test",
    srcs = ["endian_test.cc"],
    deps = [
        ":endian",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/numeric:int128",
    ],
)

cc_library(
    name = "text",
    srcs = ["text.cc"],
    hdrs = ["text.h"],
    deps = [
        ":bits",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "text_test",
    srcs = ["text_test.cc"],
    deps = [
        ":text",
        "//testing/base/public:gunit_main",
    ],
)
