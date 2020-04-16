# Copyright 2014 Google. All Rights Reserved.
# Author: jacobsa@google.com (Aaron Jacobs)

package(default_visibility = ["//visibility:private"])

licenses(["unencumbered"])  # MIT

# A binary that checks actual vs. expected output. Used by golden_test below.
cc_binary(
    name = "test_runner",
    srcs = ["test_runner.cc"],
    deps = ["//jsoncpp:json"],
)

py_test(
    name = "golden_test",
    size = "large",
    srcs = ["golden_test.py"],
    data = (
        [
            ":test_runner",
            "//jsoncpp:json",
        ] +
        # Include all test files in the data/ subdir.
        glob([
            "data/*.json",
        ]) + glob([
            "data/*.expected",
        ])
    ),
    python_version = "PY2",
    deps = ["//testing/pybase"],
)

filegroup(
    name = "json_fuzzer_corpus",
    srcs = [":data"],
    visibility = ["//jsoncpp:__subpackages__"],
)
