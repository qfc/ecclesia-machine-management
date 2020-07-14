licenses(["notice"])

cc_library(
    name = "event_logger",
    srcs = ["event_logger.cc"],
    hdrs = ["event_logger.h"],
    compatible_with = ["//buildenv/target:gce"],
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

cc_test(
    name = "event_logger_test",
    srcs = ["event_logger_test.cc"],
    deps = [
        ":event_logger",
        "//ecclesia/lib/time:clock",
        "//ecclesia/magent/lib/event_reader",
        "//ecclesia/magent/lib/event_reader:elog_emb",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
        "@com_google_emboss//runtime/cpp:cpp_utils",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "system_event_visitors",
    srcs = ["system_event_visitors.cc"],
    hdrs = ["system_event_visitors.h"],
    compatible_with = ["//buildenv/target:gce"],
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
    compatible_with = ["//buildenv/target:gce"],
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
    compatible_with = ["//buildenv/target:gce"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/apifs",
        "//ecclesia/lib/mcedecoder:cpu_topology",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "indus_cpu_topology_test",
    size = "small",
    srcs = ["indus_cpu_topology_test.cc"],
    deps = [
        "indus_cpu_topology",
        "//ecclesia/lib/apifs",
        "//ecclesia/lib/file:test_filesystem",
        "@com_google_absl//absl/status",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "indus_system_event_visitors_test",
    srcs = ["indus_system_event_visitors_test.cc"],
    deps = [
        ":event_logger",
        ":indus_system_event_visitors",
        ":system_event_visitors",
        "//ecclesia/lib/mcedecoder:cpu_topology",
        "//ecclesia/lib/time:clock",
        "//ecclesia/magent/lib/event_reader",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "system_event_visitors_test",
    srcs = ["system_event_visitors_test.cc"],
    deps = [
        ":event_logger",
        ":system_event_visitors",
        "//ecclesia/lib/mcedecoder:mce_decode",
        "//ecclesia/lib/mcedecoder:mce_messages",
        "//ecclesia/lib/time:clock",
        "//ecclesia/magent/lib/event_reader",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)
