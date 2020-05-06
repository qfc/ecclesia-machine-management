load("@com_google_emboss//:build_defs.bzl", "emboss_cc_library")

licenses(["notice"])

cc_library(
    name = "event_reader",
    hdrs = ["event_reader.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":elog_emb",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
        "@com_google_emboss//runtime/cpp:cpp_utils",
    ],
)

cc_library(
    name = "mced_reader",
    srcs = [
        "mced_reader.cc",
    ],
    hdrs = [
        "mced_reader.h",
    ],
    visibility = ["//:magent_library_users"],
    deps = [
        ":event_reader",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:variant",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "elog_reader",
    srcs = ["elog_reader.cc"],
    hdrs = ["elog_reader.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":elog_emb",
        ":event_reader",
        "//lib/smbios:reader",
        "//lib/smbios:structures_emb",
        "@com_google_emboss//runtime/cpp:cpp_utils",
    ],
)

emboss_cc_library(
    name = "elog_emb",
    srcs = ["elog.emb"],
    visibility = ["//:magent_library_users"],
)
