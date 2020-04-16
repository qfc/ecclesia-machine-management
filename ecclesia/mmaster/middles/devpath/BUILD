licenses(["notice"])

cc_library(
    name = "devpath",
    hdrs = ["devpath.h"],
    visibility = ["//:mmaster_middle_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "redfish",
    srcs = ["redfish.cc"],
    hdrs = ["redfish.h"],
    visibility = ["//:mmaster_middle_users"],
    deps = [
        ":graph_devpath",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "//lib/devpath",
        "//lib/redfish:result",
        "//lib/time:clock",
        "//mmaster/backends/redfish",
    ],
)

cc_library(
    name = "graph_devpath",
    srcs = ["graph_devpath.cc"],
    hdrs = ["graph_devpath.h"],
    visibility = ["//:mmaster_middle_users"],
    deps = [
        ":devpath",
        ":graph_generator",
        ":graph_inverter",
        ":graph_merger",
        "//base",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
        "//third_party/boost/allowed",
        "//lib/devpath",
        "//mmaster/config:cc_proto",
    ],
)

cc_library(
    name = "graph_generator",
    srcs = ["graph_generator.cc"],
    hdrs = ["graph_generator.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "//third_party/boost/allowed:allowed_headers",
        "//lib/devpath",
    ],
)

cc_library(
    name = "graph_inverter",
    srcs = ["graph_inverter.cc"],
    hdrs = ["graph_inverter.h"],
    deps = [
        ":graph_generator",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "//third_party/boost/allowed:allowed_headers",
        "//mmaster/config:cc_proto",
    ],
)

cc_library(
    name = "graph_merger",
    srcs = ["graph_merger.cc"],
    hdrs = ["graph_merger.h"],
    deps = [
        ":graph_generator",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "//third_party/boost/allowed:allowed_headers",
        "//mmaster/config:cc_proto",
    ],
)

cc_test(
    name = "graph_generator_test",
    srcs = ["graph_generator_test.cc"],
    deps = [
        ":graph_generator",
        "//testing/base/public:gunit_main",
        "//third_party/boost/allowed",
        "//third_party/boost/allowed:allowed_headers",
    ],
)

cc_test(
    name = "graph_merger_test",
    srcs = ["graph_merger_test.cc"],
    deps = [
        ":graph_generator",
        ":graph_merger",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_map",
        "//third_party/boost/allowed",
        "//mmaster/config:cc_proto",
    ],
)

cc_test(
    name = "graph_inverter_test",
    srcs = ["graph_inverter_test.cc"],
    deps = [
        ":graph_generator",
        ":graph_inverter",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_map",
        "//third_party/boost/allowed",
        "//mmaster/config:cc_proto",
    ],
)

cc_test(
    name = "graph_devpath_test",
    srcs = ["graph_devpath_test.cc"],
    deps = [
        ":graph_devpath",
        "//base",
        "//net/proto2/public:proto2",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_map",
        "//mmaster/config:cc_proto",
    ],
)
