# Define package groups for code to use to define various levels of visibility.
# Rules should always be one of:
#   * private (by default)
#   * a package group listed here
#   * public (rarely)
# If none of these are suitable for you then you may need a new group here.
# TOUCH

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
        "//ecclesia/mmaster/backends/redfish/...",
        "//ecclesia/mmaster/frontends/...",
        "//ecclesia/mmaster/middles/collector/...",
        "//ops/hardware/hydra/common/megapede_power/...",
        "//platforms/ecclesia/mmaster/backends/redfish/...",
        "//platforms/ecclesia/mmaster/frontends/...",
        "//platforms/ecclesia/mmaster/services/...",
    ],
)

# Machine agent shared libraries, for use in agent code.
package_group(
    name = "magent_library_users",
    packages = [
        "//ecclesia/magent/...",
        "//platforms/ecclesia/magent/...",
    ],
)

# Machine agent frontend code.
package_group(
    name = "magent_frontend_users",
    packages = ["//ecclesia/magent"],
)

# Allowed users of the machine master API.
package_group(
    name = "mmaster_users",
    packages = [
        "//borg/integration/...",
        "//borg/slave/...",
        "//borg/test/...",
        "//cloud/platforms/hammurabi/netboot/...",
        "//ecclesia/mmaster/...",
        "//platforms/ecclesia/lib/rpc/...",
        "//platforms/ecclesia/mmaster/...",
    ],
)

# Machine master shared libraries
package_group(
    name = "mmaster_library_users",
    packages = [
        "//ecclesia/mmaster/...",
        "//platforms/ecclesia/mmaster/...",
    ],
)

# Machine master backend code.
package_group(
    name = "mmaster_backend_users",
    packages = [
        "//ecclesia/mmaster/backends/...",
        "//ecclesia/mmaster/frontends/...",
        "//ecclesia/mmaster/middles/...",
    ],
)

# Machine master middle layer code.
package_group(
    name = "mmaster_middle_users",
    packages = [
        "//ecclesia/mmaster/frontends/...",
        "//ecclesia/mmaster/middles/...",
    ],
)

# Machine master frontend code.
package_group(
    name = "mmaster_frontend_users",
    packages = ["//ecclesia/mmaster/mock/..."],
)

# Machine master config code.
package_group(
    name = "mmaster_config_users",
    packages = ["//ecclesia/mmaster/..."],
)

# Users of the machine master mock daemon.
package_group(
    name = "mmaster_mock_users",
    packages = ["//ecclesia/mmaster/mock/..."],
)

# Tests which use the mmaster binary directly.
package_group(name = "mmaster_binary_tests")
