licenses(["notice"])

exports_files(["LICENSE.md"])

cc_library(
    name = "libredfish",
    srcs = glob(
        ["src/**"],
        exclude = ["src/main.cc"],
    ),
    hdrs = glob(["include/**"]),
    includes = ["include"],
    copts = [
        "-DNO_CZMQ",
    ],
    deps = [
        "@curl//:curl",
        "@jansson//:jansson",
    ],
    visibility = ["//visibility:public"],
)
