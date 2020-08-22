# Description:
# Library to perform minimal machine check exception decoding

licenses(["notice"])

cc_library(
    name = "interlaken_dimm_translator",
    srcs = ["dimm_translator.cc"],
    hdrs = ["dimm_translator.h"],
    visibility = ["//ecclesia/lib/mcedecoder:mcedecoder_users"],
    deps = [
        "//ecclesia/lib/mcedecoder:dimm_translator_interface",
    ],
)
