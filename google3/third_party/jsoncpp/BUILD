# -*- mode: python; -*-
#
# Copyright 2006 Google Inc. All Rights Reserved.
#
# Description:
#   jsoncpp - a C++ library for reading and writing JSON-structured data.
#
#   Please see README.google for more information.
#   Make sure to mirror any changes here (specifically to JSON_SOURCES) in the
#   json.gyp file for non-blaze clients.

load("//security/fuzzing/blaze:cc_fuzz_target.bzl", "cc_fuzz_target")

package(
    default_hdrs_check = "strict",
    default_visibility = ["//visibility:public"],
)

licenses(["unencumbered"])

exports_files(["LICENSE"])

JSON_HEADERS = [
    "assertions.h",
    "autolink.h",
    "config.h",
    "features.h",
    "forwards.h",
    "json.h",
    "reader.h",
    "value.h",
    "writer.h",
]

JSON_INTERNAL_HEADERS = [
    "json_tool.h",
    "json_valueiterator.inl.h",
    "version.h",
]

JSON_SOURCES = [
    "json_reader.cc",
    "json_value.cc",
    "json_writer.cc",
]

cc_library(
    name = "internal_json_headers",
    compatible_with = ["//buildenv/target:all"],
    textual_hdrs = JSON_INTERNAL_HEADERS,
    visibility = ["//visibility:private"],
)

cc_library(
    name = "json",
    srcs = JSON_SOURCES,
    hdrs = JSON_HEADERS,
    compatible_with = ["//buildenv/target:all"],
    copts = ["-Wno-implicit-fallthrough"],
    deps = [":internal_json_headers"],
)

# this is for applications that use "#include <json/json.h>
cc_library(
    name = "jsonsystem",
    hdrs = JSON_HEADERS,
    features = ["-parse_headers"],
    include_prefix = "json",
    deps = [":json"],
)

# If you're interested in testing JSON outputs, take a look at testing.h
cc_library(
    name = "testing",
    testonly = 1,
    srcs = [
        "testing_no_google3.h",
    ],
    hdrs = [
        "testing.h",
    ],
    deps = [
        ":json",
        "//base",
        "//testing/base/public:gunit_for_library",
        "@com_google_absl//absl/strings",
        "//util/task:status",
        "//util/task:statusor",
    ],
)

cc_library(
    name = "testing_no_google3",
    testonly = 1,
    hdrs = ["testing_no_google3.h"],
    features = ["-parse_headers"],
    deps = [
        ":json",
        "//base",
        "//testing/base/public:gunit_for_library_testonly",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "testing_test",
    size = "small",
    srcs = ["testing_test.cc"],
    deps = [
        ":json",
        ":testing",
        "//testing/base/public:gunit_main",
        "//util/task:codes_cc_proto",
    ],
)

cc_fuzz_target(
    name = "json_fuzzer",
    srcs = ["json_fuzzer.cc"],
    componentid = 125142,  # Security > ISE Tasks > TPS > Tracking
    corpus = ["//jsoncpp/golden_tests:json_fuzzer_corpus"],
    corpus_cns = [
        "/namespace/security-fuzz-testing/corpora/json-jq.ear",
        "/namespace/security-fuzz-testing/corpora/json-generated.ear",
    ],
    fuzzing_dict = ["//security/fuzzing/dictionaries:json"],
    parsers = ["json"],
    deps = [
        ":json",
        "//testing/base/public:gunit_no_heapcheck",
    ],
)

cc_fuzz_target(
    name = "jsoncpp_char_api_fuzzer",
    srcs = ["json_fuzzer_char_api.cc"],
    componentid = 125142,  # Security > ISE Tasks > TPS > Tracking
    corpus = ["//jsoncpp/golden_tests:json_fuzzer_corpus"],
    corpus_cns = [
        "/namespace/security-fuzz-testing/corpora/json-jq.ear",
        "/namespace/security-fuzz-testing/corpora/json-generated.ear",
    ],
    fuzzing_dict = ["//security/fuzzing/dictionaries:json"],
    parsers = ["json"],
    deps = [
        ":json",
        "//testing/base/public:gunit_no_heapcheck",
    ],
)

cc_fuzz_target(
    name = "json_api_fuzzer",
    srcs = ["json_api_fuzzer.cc"],
    componentid = 125142,  # Security > ISE Tasks > TPS > Tracking
    corpus = ["//jsoncpp/golden_tests:json_fuzzer_corpus"],
    corpus_cns = [
        "/namespace/security-fuzz-testing/corpora/json-jq.ear",
        "/namespace/security-fuzz-testing/corpora/json-generated.ear",
    ],
    fuzzing_dict = ["//security/fuzzing/dictionaries:json"],
    parsers = ["json"],
    deps = [
        ":json",
        "//testing/base/public:gunit_no_heapcheck",
    ],
)

cc_fuzz_target(
    name = "json_value_fuzzer",
    srcs = [
        "json_value_fuzzer.cc",
    ],
    componentid = 347144,  # Security > LASER > SunDew > AutoTargetCrashes
    corpus = ["//jsoncpp/golden_tests:json_fuzzer_corpus"],
    fuzzing_dict = ["//security/fuzzing/dictionaries:json"],
    parsers = ["json"],
    tags = ["sundew-generated"],
    deps = [
        "@com_google_absl//absl/strings",
        "//jsoncpp:json",
    ],
)

cc_fuzz_target(
    name = "json_reader_fuzz",
    srcs = [
        "json_reader_fuzz.cc",
    ],
    componentid = 347144,  # Security > LASER > SunDew > AutoTargetCrashes
    corpus = ["//jsoncpp/golden_tests:json_fuzzer_corpus"],
    fuzzing_dict = ["//security/fuzzing/dictionaries:json"],
    parsers = ["json"],
    tags = ["sundew-generated"],
    deps = [
        "@com_google_absl//absl/strings",
        "//jsoncpp:json",
    ],
)

cc_fuzz_target(
    name = "json_writer_fuzz",
    srcs = [
        "json_writer_fuzz.cc",
    ],
    componentid = 347144,  # Security > LASER > SunDew > AutoTargetCrashes
    corpus = ["//jsoncpp/golden_tests:json_fuzzer_corpus"],
    fuzzing_dict = ["//security/fuzzing/dictionaries:json"],
    parsers = ["json"],
    tags = ["sundew-generated"],
    deps = [
        "@com_google_absl//absl/strings",
        "//jsoncpp:json",
    ],
)

cc_fuzz_target(
    name = "json_styled_writer_fuzz",
    srcs = [
        "json_styled_writer_fuzz.cc",
    ],
    componentid = 347144,  # Security > LASER > SunDew > AutoTargetCrashes
    corpus = ["//jsoncpp/golden_tests:json_fuzzer_corpus"],
    fuzzing_dict = ["//security/fuzzing/dictionaries:json"],
    parsers = ["json"],
    tags = ["sundew-generated"],
    deps = [
        "@com_google_absl//absl/strings",
        "//jsoncpp:json",
    ],
)

cc_fuzz_target(
    name = "json_proto_fuzzer",
    srcs = ["json_proto_fuzzer.cc"],
    componentid = 125142,  # Security > ISE Tasks > TPS > Tracking
    fuzzing_dict = ["//security/fuzzing/dictionaries:json"],
    deps = [
        ":json",
        "//security/fuzzing/blaze:default_init_google_for_cc_fuzz_target",
        "//security/fuzzing/grammar_protos:json",
        "//testing/base/public:gunit_no_heapcheck",
        "@com_google_absl//absl/strings",
    ],
)

filegroup(
    name = "android_srcs",
    srcs = JSON_HEADERS + JSON_INTERNAL_HEADERS + JSON_SOURCES + ["Android.mk"],
)
