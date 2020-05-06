licenses(["notice"])

cc_library(
    name = "sysmodel",
    srcs = ["sysmodel.cc"],
    hdrs = ["sysmodel.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":cpu",
        ":dimm",
        ":sysmodel_fru",
        "//lib/smbios:reader",
        "//lib/time:clock",
        "//magent/lib/eeprom",
        "//magent/lib/event_logger",
        "//magent/lib/event_reader",
        "//magent/lib/event_reader:elog_reader",
        "//magent/lib/event_reader:mced_reader",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "dimm",
    srcs = ["dimm.cc"],
    hdrs = ["dimm.h"],
    visibility = ["//magent/redfish/indus:__pkg__"],
    deps = [
        "//lib/smbios:reader",
        "//lib/smbios:structures_emb",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_emboss//runtime/cpp:cpp_utils",
    ],
)

cc_library(
    name = "cpu",
    srcs = ["cpu.cc"],
    hdrs = ["cpu.h"],
    visibility = ["//magent/redfish/indus:__pkg__"],
    deps = [
        "//lib/io:msr",
        "//lib/smbios:reader",
        "//lib/smbios:structures_emb",
        "//magent/lib/event_logger:indus_cpu_topology",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_emboss//runtime/cpp:cpp_utils",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "sysmodel_fru",
    srcs = ["fru.cc"],
    hdrs = ["fru.h"],
    visibility = [
        "//magent:__pkg__",
        "//magent/redfish/indus:__pkg__",
    ],
    deps = [
        "//magent/lib/eeprom",
        "//magent/lib/fru:ipmi_fru",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)
