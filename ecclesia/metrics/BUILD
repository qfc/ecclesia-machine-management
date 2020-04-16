licenses(["notice"])

cc_library(
    name = "metrics",
    srcs = ["metrics.cc"],
    hdrs = ["metrics.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "//third_party/opencensus/exporters/stats/stdout:stdout_exporter",
        "//third_party/opencensus/stats",
        "//third_party/opencensus/tags",
    ],
)

cc_test(
    name = "metrics_test",
    size = "small",
    srcs = ["metrics_test.cc"],
    deps = [
        ":metrics",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "//third_party/opencensus/stats",
        "//third_party/opencensus/tags",
    ],
)
