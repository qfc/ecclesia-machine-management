# Description:
# Library to perform minimal machine check exception decoding

licenses(["notice"])

package_group(
    name = "mcedecoder_users",
    packages = [
        "//ecclesia/magent/...",
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

cc_library(
    name = "cpu_topology_mock",
    testonly = 1,
    hdrs = ["cpu_topology_mock.h"],
    visibility = [":mcedecoder_users"],
    deps = [
        ":cpu_topology",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "mce_decode_mock",
    testonly = 1,
    hdrs = ["mce_decode_mock.h"],
    visibility = [":mcedecoder_users"],
    deps = [
        ":mce_decode",
        ":mce_messages",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "bit_operator_test",
    srcs = ["bit_operator_test.cc"],
    deps = [
        ":bit_operator",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "mce_decode_test",
    srcs = ["mce_decode_test.cc"],
    deps = [
        ":cpu_topology",
        ":cpu_topology_mock",
        ":dimm_translator",
        ":mce_decode",
        ":mce_messages",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
    ],
)
