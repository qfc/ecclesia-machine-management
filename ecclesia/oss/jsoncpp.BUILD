#Build file for bazel workflow
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
    textual_hdrs = JSON_INTERNAL_HEADERS,
    visibility = ["//visibility:private"],
)

cc_library(
    name = "json",
    srcs = JSON_SOURCES,
    hdrs = JSON_HEADERS,
    visibility = ["//visibility:public"],
    copts = [
        "-Wno-implicit-fallthrough",
    ],
    deps = [":internal_json_headers"],
)
