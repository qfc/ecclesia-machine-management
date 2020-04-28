# Description:
#   A generic library for parsing and generating FRUs.

load("@com_google_emboss//:build_defs.bzl", "emboss_cc_library")

licenses(["notice"])

emboss_cc_library(
    name = "common_header_emb",
    srcs = ["common_header.emb"],
)

cc_library(
    name = "ipmi_fru",
    srcs = ["fru.cc"],
    hdrs = ["fru.h"],
    visibility = ["//:magent_library_users"],
    deps = [
        ":common_header_emb",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:cord",
        "@com_google_absl//absl/strings:str_format",
        "//lib/codec:text",
    ],
)

cc_test(
    name = "fru_test",
    size = "small",
    srcs = ["fru_test.cc"],
    deps = [
        ":ipmi_fru",
        "//testing/base/public:gunit_main",
    ],
)
