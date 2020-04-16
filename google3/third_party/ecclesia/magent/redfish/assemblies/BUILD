# Set of rules to package the assemblies for individual configurations

licenses(["notice"])

pkg_library(
    name = "indus_assemblies",
    srcs = glob(["indus/*.json"]),
    group = "root",
    mode = 0o444,
    owner = "root",
    package_dir = "etc/google/magent",
    visibility = [
        "//:magent_library_users",
    ],
)

pkg_library(
    name = "diorite_imc_assemblies",
    srcs = glob(["diorite_imc/*.json"]),
    group = "root",
    mode = 0o444,
    owner = "root",
    package_dir = "etc/google/magent",
    visibility = [
        "//:magent_library_users",
    ],
)
