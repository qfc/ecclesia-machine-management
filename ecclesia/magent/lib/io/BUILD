# Description:
#   Various libraries for accessing low-level buses and devices present
#   on platforms hardware.

licenses(["notice"])

cc_library(
    name = "smbus",
    srcs = ["smbus.cc"],
    hdrs = ["smbus.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "smbus_kernel_dev",
    srcs = ["smbus_kernel_dev.cc"],
    hdrs = ["smbus_kernel_dev.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":smbus",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "smbus_test",
    size = "small",
    srcs = ["smbus_test.cc"],
    deps = [
        ":smbus",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "smbus_kernel_dev_test",
    size = "small",
    srcs = ["smbus_kernel_dev_test.cc"],
    deps = [
        ":smbus_kernel_dev",
        "//platforms/util/testing:fake_filesystem",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/base:core_headers",
        "//util/gtl:singleton",
    ],
)
