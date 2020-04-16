# Copyright 2014 Google. All Rights Reserved.
# Author: jacobsa@google.com (Aaron Jacobs)

package(default_visibility = ["//visibility:private"])

licenses(["unencumbered"])  # MIT

cc_library(
    name = "jsontest",
    srcs = ["jsontest.cc"],
    hdrs = ["jsontest.h"],
    deps = [
        "//jsoncpp:json",
    ],
)

cc_test(
    name = "jsoncpp_test",
    size = "medium",
    srcs = ["jsoncpp_test.cc"],
    deps = [
        ":jsontest",
        "//jsoncpp:json",
    ],
)
