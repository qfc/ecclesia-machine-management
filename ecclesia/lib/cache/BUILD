# Description:
#   Helpful utilities for implementing caching systems.

licenses(["notice"])

package(default_visibility = ["//ecclesia:library_users"])

cc_library(
    name = "rcu",
    hdrs = [
        "rcu_snapshot.h",
        "rcu_store.h",
        "rcu_view.h",
    ],
    compatible_with = ["//buildenv/target:gce"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
    ],
)

cc_test(
    name = "rcu_snapshot_test",
    srcs = ["rcu_snapshot_test.cc"],
    deps = [
        ":rcu",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "rcu_store_test",
    srcs = ["rcu_store_test.cc"],
    deps = [
        ":rcu",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "rcu_view_test",
    srcs = ["rcu_view_test.cc"],
    deps = [
        ":rcu",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
