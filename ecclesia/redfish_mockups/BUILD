# BUILD rules for mockup servers. No other BUILD rules other than
# redfish_mockup should be defined in this file.

load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":build_defs.bzl", "redfish_mockup")

licenses(["notice"])

bzl_library(
    name = "build_defs",
    srcs = ["build_defs.bzl"],
    deps = ["//ecclesia/build_defs:embed"],
)

redfish_mockup(
    name = "indus_hmb_cn_mockup.shar",
    datafile_dir = "indus_hmb_cn",
    visibility = [
        "//ecclesia:mmanager_binary_tests",
        "//ecclesia:mmanager_middle_users",
        "//ecclesia:redfish_users",
        "//platforms/ecclesia/mmanager/mock/mockups:mmanager_mockups",
    ],
)

redfish_mockup(
    name = "interlaken_hmb_cn_mockup.shar",
    datafile_dir = "interlaken_hmb_cn",
    visibility = [
        "//ecclesia:mmanager_binary_tests",
        "//ecclesia:mmanager_middle_users",
        "//ecclesia:redfish_users",
    ],
)

redfish_mockup(
    name = "skyholm_plus_mockup.shar",
    datafile_dir = "skyholm_plus",
    visibility = [
        "//ecclesia:mmanager_binary_tests",
        "//ecclesia:mmanager_middle_users",
        "//ecclesia:redfish_users",
    ],
)

redfish_mockup(
    name = "sleipnir_mockup.shar",
    datafile_dir = "sleipnir",
    visibility = [
        "//ecclesia:redfish_users",
    ],
)
