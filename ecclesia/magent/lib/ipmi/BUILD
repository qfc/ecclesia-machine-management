# IPMI interface libraries for management agent

licenses(["notice"])

cc_library(
    name = "ipmi",
    hdrs = ["ipmi.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "interface_options",
    srcs = ["interface_options.cc"],
    hdrs = ["interface_options.h"],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/cleanup",
        "//ecclesia/lib/logging",
        "//ecclesia/magent:magent_config_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "ipmitool",
    srcs = [
        "ipmitool.cc",
    ],
    hdrs = [
        "ipmitool.h",
    ],
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        ":interface_options",
        ":ipmi",
        "//ecclesia/lib/logging",
        "//ecclesia/magent:magent_config_cc_proto",
        "//ecclesia/magent/lib/fru:ipmi_fru",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@ipmitool//:ipmi",
    ],
)
