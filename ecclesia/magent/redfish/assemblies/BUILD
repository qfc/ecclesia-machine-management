# Set of rules to package the assemblies for individual configurations

load("@rules_pkg//:pkg.bzl", "pkg_tar")

licenses(["notice"])

pkg_tar(
    name = "indus_assemblies_tar",
    srcs = glob(["indus/*.json"]),
    extension = "tar.gz",
    strip_prefix = ".",
    visibility = [
        "//ecclesia:magent_library_users",
    ],
)

pkg_tar(
    name = "interlaken_assemblies_tar",
    srcs = glob(["interlaken/*.json"]),
    extension = "tar.gz",
    strip_prefix = ".",
    visibility = [
        "//ecclesia:magent_library_users",
    ],
)
