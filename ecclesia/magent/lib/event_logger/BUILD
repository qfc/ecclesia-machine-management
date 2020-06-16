licenses(["notice"])

cc_library(
    name = "event_logger",
    srcs = ["event_logger.cc"],
    hdrs = ["event_logger.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/time:clock",
        "//ecclesia/magent/lib/event_reader",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "system_event_visitors",
    srcs = ["system_event_visitors.cc"],
    hdrs = ["system_event_visitors.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        ":event_logger",
        "//ecclesia/lib/mcedecoder:mce_decode",
        "//ecclesia/lib/mcedecoder:mce_messages",
        "//ecclesia/magent/lib/event_reader",
        "//ecclesia/magent/lib/event_reader:elog_emb",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
        "@com_google_emboss//runtime/cpp:cpp_utils",
    ],
)

cc_library(
    name = "indus_system_event_visitors",
    srcs = ["indus_system_event_visitors.cc"],
    hdrs = ["indus_system_event_visitors.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        ":system_event_visitors",
        "//ecclesia/lib/mcedecoder:cpu_topology",
        "//ecclesia/lib/mcedecoder:dimm_translator",
        "//ecclesia/lib/mcedecoder:mce_decode",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "indus_cpu_topology",
    srcs = ["indus_cpu_topology.cc"],
    hdrs = ["indus_cpu_topology.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/apifs",
        "//ecclesia/lib/mcedecoder:cpu_topology",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)
