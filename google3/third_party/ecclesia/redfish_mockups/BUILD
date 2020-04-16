# BUILD rules for mockup servers. No other BUILD rules other than
# redfish_mockup should be defined in this file.

load(":build_defs.bzl", "redfish_mockup")

licenses(["notice"])

redfish_mockup(
    name = "indus_hmb_cn_mockup",
    datafile_dir = "indus_hmb_cn",
    visibility = [
        "//:mmaster_binary_tests",
        "//:mmaster_middle_users",
        "//:redfish_users",
    ],
)
