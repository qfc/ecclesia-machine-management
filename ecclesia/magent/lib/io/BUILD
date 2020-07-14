# Description:
#   Various libraries for accessing low-level buses and devices present
#   on platforms hardware.

licenses(["notice"])

cc_library(
    name = "smbus",
    hdrs = ["smbus.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/types:fixed_range_int",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "smbus_mocks",
    testonly = 1,
    hdrs = ["smbus_mocks.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        ":smbus",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "smbus_kernel_dev",
    srcs = ["smbus_kernel_dev.cc"],
    hdrs = ["smbus_kernel_dev.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        ":smbus",
        "//ecclesia/lib/cleanup",
        "//ecclesia/lib/io:ioctl",
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
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "smbus_kernel_dev_test",
    size = "small",
    srcs = ["smbus_kernel_dev_test.cc"],
    deps = [
        ":smbus",
        ":smbus_kernel_dev",
        "//ecclesia/lib/file:test_filesystem",
        "//ecclesia/lib/io:ioctl",
        "//ecclesia/lib/logging",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "pci_location",
    srcs = ["pci_location.cc"],
    hdrs = ["pci_location.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/types:fixed_range_int",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_test(
    name = "pci_location_test",
    size = "small",
    srcs = ["pci_location_test.cc"],
    deps = [
        ":pci_location",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "pci",
    hdrs = [
        "pci.h",
        "pci_consts.h",
        "pci_regs.h",
    ],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        ":pci_location",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "pci_cc",
    hdrs = ["pci.h"],
    deps = [
        ":pci",
        ":pci_location",
        "@com_google_absl//absl/status",
    ],
)

cc_library(
    name = "pci_sys",
    srcs = ["pci_sys.cc"],
    hdrs = ["pci_sys.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        ":pci",
        ":pci_location",
        "//ecclesia/lib/apifs",
        "//ecclesia/lib/codec:endian",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "pci_sys_test",
    size = "small",
    srcs = ["pci_sys_test.cc"],
    deps = [
        ":pci_sys",
        "//ecclesia/lib/file:test_filesystem",
        "//ecclesia/lib/io:ioctl",
        "//ecclesia/lib/logging",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)
