licenses(["notice"])

config_setting(
    name = "external_build",
    values = {"define": "build=external"},
    visibility = ["//ecclesia:magent_library_users"],
)

proto_library(
    name = "magent_config_proto",
    srcs = ["config.proto"],
    visibility = ["//ecclesia:magent_library_users"],
)

cc_proto_library(
    name = "magent_config_cc_proto",
    visibility = ["//ecclesia:magent_library_users"],
    deps = [":magent_config_proto"],
)

cc_binary(
    name = "magent_indus_oss",
    srcs = ["main_indus.cc"],
    visibility = [
        "//ecclesia:magent_library_users",
    ],
    deps = [
        ":magent_hdr",
        "//ecclesia/lib/apifs",
        "//ecclesia/lib/io:ioctl",
        "//ecclesia/lib/smbios:reader",
        "//ecclesia/lib/types:fixed_range_int",
        "//ecclesia/magent/lib/eeprom",
        "//ecclesia/magent/lib/io:pci_location",
        "//ecclesia/magent/lib/io:smbus",
        "//ecclesia/magent/lib/io:smbus_kernel_dev",
        "//ecclesia/magent/lib/ipmi:interface_options",
        "//ecclesia/magent/lib/ipmi:ipmitool",
        "//ecclesia/magent/redfish/indus",
        "//ecclesia/magent/sysmodel/x86:sysmodel_fru",
        "//ecclesia/magent/sysmodel/x86:thermal",
        "//ecclesia/magent/sysmodel/x86:x86_sysmodel",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server_api",
    ],
)

cc_library(
    name = "magent_hdr",
    hdrs = ["main_common.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/magent/lib/thread_pool",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server_api",
    ],
)
