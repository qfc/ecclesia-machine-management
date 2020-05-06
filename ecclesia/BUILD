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
        "//...",
        "//platforms/ecclesia/...",
        "//third_party/blt/...",
    ],
)

# Code intended for use in the Redfish backend.
package_group(
    name = "redfish_users",
    packages = [
        "//lib/redfish/...",
        "//mmaster/backends/redfish/...",
        "//mmaster/frontends/...",
        "//platforms/ecclesia/mmaster/backends/redfish/...",
        "//platforms/ecclesia/mmaster/frontends/...",
    ],
)

# Machine agent shared libraries, for use in agent code.
package_group(
    name = "magent_library_users",
    packages = [
        "//magent/...",
        "//platforms/ecclesia/magent/...",
    ],
)

# Machine agent frontend code.
package_group(
    name = "magent_frontend_users",
    packages = ["//magent"],
)

# Allowed users of the machine master API.
package_group(
    name = "mmaster_users",
    packages = [
        "//cloud/platforms/hammurabi/netboot/...",
        "//mmaster/...",
        "//platforms/ecclesia/lib/rpc/...",
        "//platforms/ecclesia/mmaster/...",
    ],
)

# Machine master backend code.
package_group(
    name = "mmaster_backend_users",
    packages = [
        "//mmaster/middles/...",
    ],
)

# Machine master middle layer code.
package_group(
    name = "mmaster_middle_users",
    packages = [
        "//mmaster/frontends/...",
        "//mmaster/middles/...",
    ],
)

# Machine master frontend code.
package_group(
    name = "mmaster_frontend_users",
    packages = ["//mmaster/mock/..."],
)

# Machine master config code.
package_group(
    name = "mmaster_config_users",
    packages = ["//mmaster/..."],
)

# Users of the machine master mock daemon.
package_group(
    name = "mmaster_mock_users",
    packages = ["//mmaster/mock/..."],
)

# Tests which use the mmaster binary directly.
package_group(name = "mmaster_binary_tests")
