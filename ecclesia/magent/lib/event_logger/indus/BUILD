licenses(["notice"])

cc_library(
    name = "indus_system_event_visitors",
    srcs = ["system_event_visitors.cc"],
    hdrs = ["system_event_visitors.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/mcedecoder:cpu_topology",
        "//ecclesia/lib/mcedecoder:mce_decode",
        "//ecclesia/lib/mcedecoder/indus:indus_dimm_translator",
        "//ecclesia/magent/lib/event_logger:system_event_visitors",
        "@com_google_absl//absl/time",
    ],
)

cc_test(
    name = "indus_system_event_visitors_test",
    srcs = ["system_event_visitors_test.cc"],
    deps = [
        ":indus_system_event_visitors",
        "//ecclesia/lib/mcedecoder:cpu_topology",
        "//ecclesia/lib/time:clock",
        "//ecclesia/magent/lib/event_logger",
        "//ecclesia/magent/lib/event_logger:system_event_visitors",
        "//ecclesia/magent/lib/event_reader",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)
