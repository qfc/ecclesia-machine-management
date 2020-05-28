# Description:
#   Helpful utilities for implementing caching systems.

licenses(["notice"])

package(default_visibility = ["//:library_users"])

cc_library(
    name = "rcu",
    hdrs = [
        "rcu_snapshot.h",
        "rcu_store.h",
        "rcu_view.h",
    ],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
    ],
)
