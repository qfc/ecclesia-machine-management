# Description:
#   Various libraries for accessing low-level buses and devices present
#   on platforms hardware.

licenses(["notice"])

cc_library(
    name = "smbus",
    hdrs = ["smbus.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        "//lib/types:fixed_range_int",
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
        "//lib/cleanup",
        "//lib/io:ioctl",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "pci_location",
    srcs = ["pci_location.cc"],
    hdrs = ["pci_location.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        "//lib/types:fixed_range_int",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_googlesource_code_re2//:re2",
    ],
)
