licenses(["notice"])

cc_library(
    name = "task",
    hdrs = ["task.h"],
    deps = ["@com_google_absl//absl/time"],
)

cc_test(
    name = "task_test",
    srcs = ["task_test.cc"],
    deps = [
        ":task",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "manager",
    hdrs = ["manager.h"],
    deps = [":task"],
)

cc_test(
    name = "manager_test",
    srcs = ["manager_test.cc"],
    deps = [
        ":manager",
        ":task",
        "//testing/base/public:gunit_main",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/time",
    ],
)
