# Description:
#   Libraries for accessing Eeprom

licenses(["notice"])

package(default_visibility = ["//ecclesia:magent_library_users"])

cc_library(
    name = "eeprom",
    srcs = ["smbus_eeprom.cc"],
    hdrs = [
        "eeprom.h",
        "smbus_eeprom.h",
    ],
    deps = [
        "//ecclesia/lib/logging",
        "//ecclesia/magent/lib/io:smbus",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "smbus_eeprom_test",
    size = "small",
    srcs = ["smbus_eeprom_test.cc"],
    deps = [
        ":eeprom",
        "//ecclesia/magent/lib/io:smbus",
        "//ecclesia/magent/lib/io:smbus_mocks",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)
