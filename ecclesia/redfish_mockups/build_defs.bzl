"""Build rules for generating mockups"""

load("//ecclesia/build_defs:embed.bzl", "shar_binary")

def redfish_mockup(name, datafile_dir, visibility = None):
    """Generate redfish mockup bash rules.

    Note that the output of this rule is a shar_binary, so for consistency we require
    that the name end with .shar.

    Args:
        name: Name of build target. Must be distinct from datafile_dir.
        datafile_dir: Directory containing all mockup data files.
        visibility: Visibility to apply to the resulting sh_binary build target.
    """
    if not name.endswith(".shar"):
        fail("A redfish_mockup build rule name must end with .shar")
    if name == datafile_dir:
        fail("The redfish_mockup build rule must not have identical 'name' and 'datafile_dir' fields.")

    datafile_glob_str = datafile_dir + "/**"
    datafile_glob_files = native.glob([datafile_glob_str])
    shar_binary(
        name = name,
        src = "serve_mockup.sh",
        data = ["@redfishMockupServer//:redfishMockupServer.par"] +
               datafile_glob_files,
        visibility = visibility,
    )
