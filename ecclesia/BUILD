# Define package groups for code to use to define various levels of visibility.
# Rules should always be one of:
#   * private (by default)
#   * a package group listed here
#   * public (rarely)
# If none of these are suitable for you then you may need a new group here.

licenses(["notice"])

exports_files(["LICENSE"])

# Code allowed to use any of our generic libraries.
package_group(
    name = "library_users",
    packages = [
        "//ecclesia/...",
        "//platforms/ecclesia/...",
        "//platforms/gsys/...",
        "//third_party/blt/...",
    ],
)

# Code intended for use in the Redfish backend.
package_group(
    name = "redfish_users",
    packages = [
        "//ecclesia/lib/redfish/...",
        "//ecclesia/magent/redfish/...",
        "//ecclesia/mmanager/backends/redfish/...",
        "//ecclesia/mmanager/frontends/...",
        "//ecclesia/mmanager/middles/collector/...",
        "//ops/hardware/hydra/common/megapede_power/...",
        "//platforms/ecclesia/mmanager/backends/redfish/...",
        "//platforms/ecclesia/mmanager/frontends/...",
        "//platforms/ecclesia/mmanager/services/...",
    ],
)

# Machine agent shared libraries, for use in agent code.
package_group(
    name = "magent_library_users",
    packages = [
        "//ecclesia/magent/...",
        "//platforms/ecclesia/assembly_generator/...",
        "//platforms/ecclesia/magent/...",
    ],
)

# Machine agent frontend code.
package_group(
    name = "magent_frontend_users",
    packages = ["//ecclesia/magent"],
)

# Allowed users of the machine manager API.
package_group(
    name = "mmanager_users",
    packages = [
        "//borg/integration/...",
        "//borg/slave/...",
        "//borg/test/...",
        "//cloud/platforms/hammurabi/netboot/...",
        "//ecclesia/mmanager/...",
        "//platforms/ecclesia/lib/rpc/...",
        "//platforms/ecclesia/mmanager/...",
        "//platforms/syshealth/...",
        "//production/aristotle/...",
    ],
)

# Machine manager shared libraries
package_group(
    name = "mmanager_library_users",
    packages = [
        "//ecclesia/mmanager/...",
        "//platforms/ecclesia/mmanager/...",
    ],
)

# Machine manager backend code.
package_group(
    name = "mmanager_backend_users",
    packages = [
        "//ecclesia/mmanager/backends/...",
        "//ecclesia/mmanager/frontends/...",
        "//ecclesia/mmanager/middles/...",
    ],
)

# Machine manager middle layer code.
package_group(
    name = "mmanager_middle_users",
    packages = [
        "//ecclesia/mmanager/frontends/...",
        "//ecclesia/mmanager/middles/...",
    ],
)

# Machine manager frontend code.
package_group(
    name = "mmanager_frontend_users",
    packages = ["//ecclesia/mmanager/mock/..."],
)

# Machine manager config code.
package_group(
    name = "mmanager_config_users",
    packages = ["//ecclesia/mmanager/..."],
)

# Users of the machine manager mock daemon.
package_group(
    name = "mmanager_mock_users",
    packages = ["//ecclesia/mmanager/mock/..."],
)

# Tests which use the mmanager binary directly.
package_group(name = "mmanager_binary_tests")
