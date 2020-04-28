licenses(["notice"])

# TODO(jaghu) : Document common patterns (and maybe even add some bazel macros if there are particular types of standard rules we want to define).
# TODO(jaghu) : The magent is intended to be buildable outside of piper, and sharable with vendors. If there are going to be variants of it which should _not_ be shared, then we'll need some way to handle this (e.g. make sure we only include a whitelist of stuff in the copybara?)
cc_binary(
    name = "magent_indus",
    srcs = ["main_indus.cc"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":magent_hdr",
        "//lib/smbios:reader",
        "//magent/redfish/indus",
        "//magent/sysmodel",
        "//magent/sysmodel:fru",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/memory",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server_api",
    ],
)

cc_binary(
    name = "magent_diorite_acc",
    srcs = ["main_diorite_acc.cc"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":magent_hdr",
        "//magent/redfish/diorite",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/memory",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server_api",
    ],
)

cc_binary(
    name = "magent_diorite_imc",
    srcs = ["main_diorite_imc.cc"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":magent_hdr",
        "//magent/redfish/diorite",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/memory",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server_api",
    ],
)

cc_library(
    name = "magent_hdr",
    hdrs = ["main_common.h"],
    deps = [
        "//magent/lib/thread_pool",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server_api",
    ],
)
