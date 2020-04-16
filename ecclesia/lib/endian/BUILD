licenses(["notice"])

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
