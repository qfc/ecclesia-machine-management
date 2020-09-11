licenses(["notice"])

cc_library(
    name = "redfish_core",
    srcs = [
        "assembly.cc",
    ],
    hdrs = [
        "assembly.h",
        "index_resource.h",
        "json_helper.h",
        "odata_metadata.h",
        "redfish_keywords.h",
        "resource.h",
        "service_root_resource.h",
    ],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/logging",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:variant",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server_api",
        "@com_googlesource_code_re2//:re2",
        "@com_jsoncpp//:json",
    ],
)
