# Description:
# Library to perform minimal machine check exception decoding

licenses(["notice"])

cc_library(
    name = "indus_dimm_translator",
    srcs = ["dimm_translator.cc"],
    hdrs = ["dimm_translator.h"],
    visibility = ["//ecclesia/lib/mcedecoder:mcedecoder_users"],
    deps = [
        "//ecclesia/lib/mcedecoder:dimm_translator_interface",
    ],
)

cc_test(
    name = "mce_decode_test",
    srcs = ["mce_decode_test.cc"],
    deps = [
        "//ecclesia/lib/mcedecoder:cpu_topology",
        "//ecclesia/lib/mcedecoder:cpu_topology_mock",
        "//ecclesia/lib/mcedecoder:mce_decode",
        "//ecclesia/lib/mcedecoder:mce_messages",
        "//ecclesia/lib/mcedecoder/indus:indus_dimm_translator",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
    ],
)
