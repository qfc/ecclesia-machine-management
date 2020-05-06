licenses(["notice"])

cc_library(
    name = "task",
    hdrs = ["task.h"],
    visibility = ["//:library_users"],
    deps = ["@com_google_absl//absl/time"],
)

cc_library(
    name = "manager",
    hdrs = ["manager.h"],
    visibility = ["//:library_users"],
    deps = [":task"],
)
