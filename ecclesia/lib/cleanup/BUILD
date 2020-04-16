# Description: Simple header only utility library that provide RAII cleanup objects

licenses(["notice"])

# TODO(b/154161341): Remove this library once absl::MakeCleanup() is available
# in the open source version of absl.
cc_library(
    name = "cleanup",
    hdrs = ["cleanup.h"],
    visibility = ["//:library_users"],
)
