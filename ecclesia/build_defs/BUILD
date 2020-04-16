load("//third_party/bazel_skylib:bzl_library.bzl", "bzl_library")

licenses(["notice"])

bzl_library(
    name = "file",
    srcs = ["file.bzl"],
)

bzl_library(
    name = "constants",
    srcs = ["constants.bzl"],
    deps = [":file"],
)
