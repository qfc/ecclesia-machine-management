licenses(["notice"])

cc_library(
    name = "sysmodel",
    srcs = ["sysmodel.cc"],
    hdrs = ["sysmodel.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":cpu",
        ":dimm",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/types:optional",
        "//lib/time:clock",
        "//magent/lib/event_logger",
        "//magent/lib/event_reader",
        "//magent/lib/event_reader:elog_reader",
        "//magent/lib/event_reader:mced_reader",
        "//magent/lib/smbios:reader",
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
        "//magent/lib/smbios:reader",
        "//magent/lib/smbios:structures_emb",
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
        "//magent/lib/event_logger:indus_cpu_topology",
        "//magent/lib/smbios:reader",
        "//magent/lib/smbios:structures_emb",
        "@com_google_emboss//runtime/cpp:cpp_utils",
        "@com_googlesource_code_re2//:re2",
    ],
)
