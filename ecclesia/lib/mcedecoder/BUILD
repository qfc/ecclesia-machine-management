# Description:
# Library to perform minimal machine check exception decoding

licenses(["notice"])

package_group(
    name = "mcedecoder_users",
    packages = [
        "//magent/...",
        "//platforms/baremetal/healthagent/...",
    ],
)

cc_library(
    name = "cpu_topology",
    hdrs = ["cpu_topology.h"],
    visibility = [":mcedecoder_users"],
)

cc_library(
    name = "dimm_translator",
    srcs = ["dimm_translator.cc"],
    hdrs = ["dimm_translator.h"],
    visibility = [":mcedecoder_users"],
)

cc_library(
    name = "mce_messages",
    hdrs = ["mce_messages.h"],
    visibility = [":mcedecoder_users"],
)

cc_library(
    name = "bit_operator",
    hdrs = ["bit_operator.h"],
)

cc_library(
    name = "mce_decode",
    srcs = ["mce_decode.cc"],
    hdrs = ["mce_decode.h"],
    visibility = [":mcedecoder_users"],
    deps = [
        ":bit_operator",
        ":cpu_topology",
        ":dimm_translator",
        ":mce_messages",
        ":skylake_mce_decode",
    ],
)

cc_library(
    name = "skylake_mce_decode",
    srcs = ["skylake_mce_decode.cc"],
    hdrs = ["skylake_mce_decode.h"],
    deps = [
        ":bit_operator",
        ":dimm_translator",
        ":mce_messages",
    ],
)
