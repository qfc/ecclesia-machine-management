"""Build rules for generating mockups"""

load("//tools/build_defs/pkg:pkg.bzl", "pkg_tar")
load("//tools/build_defs/docker:docker.bzl", "docker_build")

def redfish_mockup(name, datafile_dir, visibility = None):
    """Generate redfish mockup bash rules.

    Rules generated:
        ${name}: sh binary
        ${name}.sar: .sar binary
        ${name}.img: docker image

    Args:
        name: Name of build target. Must be distinct from datafile_dir.
        datafile_dir: Directory containing all mockup data files.
        visibility: Visibility to apply to the resulting sh_binary build target.
    """
    if name == datafile_dir:
        fail('The redfish_mockup build rule must not have identical "name" and "datafile_dir" fields.')

    datafile_glob_str = datafile_dir + "/**"
    datafile_glob_files = native.glob([datafile_glob_str])
    native.sh_binary(
        name = name,
        bash_version = "4",
        srcs = ["mockup.sh"],
        data = datafile_glob_files +
               ["//third_party/py/redfishMockupServer:redfishMockupServer.par"],
        deps = ["//util/shell/gbash"],
        visibility = visibility,
    )

    tarname = "_" + name + "-data"
    dockername = name + ".img"

    pkg_tar(
        name = tarname,
        srcs = datafile_glob_files,
        extension = "tar.gz",
        strip_prefix = ".",
    )

    docker_build(
        name = dockername,
        base = "//third_party/grte/docker:grte-base-image",
        entrypoint = [
            "/redfishMockupServer.par",
            "-D",
            "/" + datafile_dir,
            "-H",
            "0.0.0.0",
        ],
        files = [
            "//third_party/py/redfishMockupServer:redfishMockupServer.par",
        ],
        ports = ["8000"],
        tars = [
            ":" + tarname,
        ],
    )
