licenses(["notice"])

cc_library(
    name = "x86_sysmodel",
    srcs = ["sysmodel.cc"],
    hdrs = ["sysmodel.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        ":cpu",
        ":dimm",
        ":sysmodel_fru",
        ":thermal",
        "//ecclesia/lib/smbios:reader",
        "//ecclesia/lib/time:clock",
        "//ecclesia/magent/lib/eeprom",
        "//ecclesia/magent/lib/event_logger",
        "//ecclesia/magent/lib/event_reader",
        "//ecclesia/magent/lib/event_reader:elog_reader",
        "//ecclesia/magent/lib/event_reader:mced_reader",
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
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/smbios:reader",
        "//ecclesia/lib/smbios:structures_emb",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_emboss//runtime/cpp:cpp_utils",
    ],
)

cc_library(
    name = "cpu",
    srcs = ["cpu.cc"],
    hdrs = ["cpu.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/io:msr",
        "//ecclesia/lib/smbios:reader",
        "//ecclesia/lib/smbios:structures_emb",
        "//ecclesia/magent/lib/event_logger:indus_cpu_topology",
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
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/magent/lib/eeprom",
        "//ecclesia/magent/lib/fru:ipmi_fru",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "thermal",
    srcs = ["thermal.cc"],
    hdrs = ["thermal.h"],
    visibility = ["//ecclesia:magent_library_users"],
    deps = [
        "//ecclesia/lib/io:msr",
        "//ecclesia/magent/lib/event_logger:indus_cpu_topology",
        "//ecclesia/magent/lib/io:pci",
        "//ecclesia/magent/lib/io:pci_location",
        "//ecclesia/magent/lib/io:pci_sys",
        "//ecclesia/magent/sysmodel:thermal",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)
