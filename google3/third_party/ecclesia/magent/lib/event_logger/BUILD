licenses(["notice"])

cc_library(
    name = "event_logger",
    srcs = ["event_logger.cc"],
    hdrs = ["event_logger.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "//lib/time:clock",
        "//magent/lib/event_reader",
    ],
)

cc_test(
    name = "event_logger_test",
    srcs = ["event_logger_test.cc"],
    deps = [
        ":event_logger",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "//lib/time:clock",
        "//magent/lib/event_reader",
        "//magent/lib/event_reader:elog_emb",
        "@com_google_emboss//runtime/cpp:cpp_utils",
    ],
)

cc_library(
    name = "system_event_visitors",
    srcs = ["system_event_visitors.cc"],
    hdrs = ["system_event_visitors.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":event_logger",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/time",
        "//lib/mcedecoder:mce_decode",
        "//lib/mcedecoder:mce_messages",
        "//magent/lib/event_reader",
        "//magent/lib/event_reader:elog_emb",
        "@com_google_emboss//runtime/cpp:cpp_utils",
    ],
)

cc_library(
    name = "indus_system_event_visitors",
    srcs = ["indus_system_event_visitors.cc"],
    hdrs = ["indus_system_event_visitors.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":indus_cpu_topology",
        ":system_event_visitors",
        "@com_google_absl//absl/time",
        "//lib/mcedecoder:cpu_topology",
        "//lib/mcedecoder:dimm_translator",
        "//lib/mcedecoder:mce_decode",
    ],
)

cc_library(
    name = "indus_cpu_topology",
    srcs = ["indus_cpu_topology.cc"],
    hdrs = ["indus_cpu_topology.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "//lib/apifs",
        "//lib/mcedecoder:cpu_topology",
    ],
)

cc_test(
    name = "indus_cpu_topology_test",
    size = "small",
    srcs = ["indus_cpu_topology_test.cc"],
    deps = [
        "indus_cpu_topology",
        "//platforms/util/testing:fake_filesystem",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "indus_system_event_visitors_test",
    srcs = ["indus_system_event_visitors_test.cc"],
    deps = [
        ":event_logger",
        ":indus_cpu_topology",
        ":indus_system_event_visitors",
        ":system_event_visitors",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "//lib/apifs",
        "//lib/time:clock",
        "//magent/lib/event_reader",
    ],
)

cc_test(
    name = "system_event_visitors_test",
    srcs = ["system_event_visitors_test.cc"],
    deps = [
        ":event_logger",
        ":system_event_visitors",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "//lib/mcedecoder:mce_decode",
        "//lib/mcedecoder:mce_messages",
        "//lib/time:clock",
        "//magent/lib/event_reader",
    ],
)
