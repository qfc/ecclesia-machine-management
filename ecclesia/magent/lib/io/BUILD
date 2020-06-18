# Description:
#   Various libraries for accessing low-level buses and devices present
#   on platforms hardware.

licenses(["notice"])

cc_library(
    name = "smbus",
    hdrs = ["smbus.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/types:fixed_range_int",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "smbus_kernel_dev",
    srcs = ["smbus_kernel_dev.cc"],
    hdrs = ["smbus_kernel_dev.h"],
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

cc_library(
    name = "pci_location",
    srcs = ["pci_location.cc"],
    hdrs = ["pci_location.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/types:fixed_range_int",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "pci",
    hdrs = [
        "pci.h",
        "pci_consts.h",
        "pci_regs.h",
    ],
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
