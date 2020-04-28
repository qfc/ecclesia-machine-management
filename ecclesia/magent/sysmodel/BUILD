licenses(["notice"])

cc_library(
    name = "sysmodel",
    srcs = ["sysmodel.cc"],
    hdrs = ["sysmodel.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":cpu",
        ":dimm",
        ":fru",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
        "//lib/smbios:reader",
        "//lib/time:clock",
        "//magent/lib/event_logger",
        "//magent/lib/event_reader",
        "//magent/lib/event_reader:elog_reader",
        "//magent/lib/event_reader:mced_reader",
    ],
)

cc_library(
    name = "dimm",
    srcs = ["dimm.cc"],
    hdrs = ["dimm.h"],
    visibility = ["//magent/redfish/indus:__pkg__"],
    deps = [
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "//lib/smbios:reader",
        "//lib/smbios:structures_emb",
        "@com_google_emboss//runtime/cpp:cpp_utils",
    ],
)

cc_library(
    name = "cpu",
    srcs = ["cpu.cc"],
    hdrs = ["cpu.h"],
    visibility = ["//magent/redfish/indus:__pkg__"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "//lib/io:msr",
        "//lib/smbios:reader",
        "//lib/smbios:structures_emb",
        "//magent/lib/event_logger:indus_cpu_topology",
        "@com_google_emboss//runtime/cpp:cpp_utils",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "fru",
    srcs = ["fru.cc"],
    hdrs = ["fru.h"],
    visibility = [
        "//magent/redfish/indus:__pkg__",
        # TODO(dwangsf): Remove this visibility rule once we no longer need to harcode FRU info
        "//magent:__pkg__",
    ],
    deps = [
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)
