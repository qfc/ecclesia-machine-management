"""Starlark definitions for build rules to generate cross-language constants.

This file defines macros that can be used to define integer and string constants
which can then be embedded into language-specifc libraries. For more complex
data a more structured format and embedding mechanism should be used but for
very simple numeric and string data this provides a much simpler and lighter
weight mechanism that something like protocol buffers.
"""

load("//third_party/bazel_skylib/lib:types.bzl", "types")
load(":file.bzl", "write_file")

def constant(
        value,
        cc_name,
        go_name,
        py_name):
    """Define a single constant value.

    Args:
      value: The value of the constant. Can be an integer or string.
      cc_name: The name of the constant in C++.
      go_name: The name of the constant in Go.
      py_name: The name of the constant in Python.
    Returns:
      A struct that can be passed to constant_lib.
    """
    if types.is_int(value):
        c_type = "int"
    elif types.is_string(value):
        c_type = "string"
    else:
        fail("Only integer and string constants are allowed")
    return struct(
        value_type = c_type,
        value = value,
        cc_name = cc_name,
        go_name = go_name,
        py_name = py_name,
    )

def constant_lib(
        name,
        cc_namespace,
        constants,
        visibility):
    """Generates a language libraries containing the provided constants.

    The library for each language will be generated by taking the given name and
    appending a standard suffix to it:
      C++: ${name}_cc
      Go: ${name}_go
      Python: ${name}_py
    The name will also be used to define the filename of the generated file that
    contains the constants:
      C++: ${name}.h
      Go: ${name}.go
      Python: ${name}.py

    Args:
      name: The base name for the generated libraries.
      cc_namespace: The C++ namespace that the constants will be enclosed in.
      constants: A list of "constant" objects, one for each constant.
      visibility: The visibility of all of the generated libraries.
    """

    # Generate an include guard to apply to the C++ header.
    cc_include_guard = "%s_%s_H_" % (
        native.package_name().replace("/", "_").upper(),
        name.upper(),
    )

    # Construct the code for all of the different languages.
    cc_lines = [
        "// This code was generated by //%s:%s" % (native.package_name(), name),
        "#ifndef %s" % cc_include_guard,
        "#define %s" % cc_include_guard,
        "",
        "#include <string_view>",
        "",
        "namespace %s {" % cc_namespace,
    ]
    go_lines = [
        "// This code was generated by //%s:%s" % (native.package_name(), name),
        "package %s" % name,
        "const (",
    ]
    py_lines = [
        "# This code was generated by //%s:%s" % (native.package_name(), name),
    ]
    for constant in constants:
        if constant.value_type == "int":
            cc_lines.append("inline constexpr int %s = %d;" %
                            (constant.cc_name, constant.value))
            go_lines.append("  %s int = %d" %
                            (constant.go_name, constant.value))
            py_lines.append("%s = %d" %
                            (constant.py_name, constant.value))
        elif constant.value_type == "string":
            cc_lines.append("inline constexpr std::string_view %s = \"%s\";" %
                            (constant.cc_name, constant.value))
            go_lines.append("  %s string = \"%s\"" %
                            (constant.go_name, constant.value))
            py_lines.append("%s = '%s'" %
                            (constant.py_name, constant.value))
    cc_lines.extend([
        "}  // namespace %s" % cc_namespace,
        "#endif  // %s" % cc_include_guard,
    ])
    go_lines.append(")")

    # Generate the C++ library.
    cc_write_file = "%s.cc.write_file" % name
    cc_header = "%s.h" % name
    cc_name = "%s_cc" % name
    write_file(
        name = cc_write_file,
        content = "\n".join(cc_lines),
        out = cc_header,
    )
    native.cc_library(
        name = cc_name,
        hdrs = [cc_header],
        visibility = visibility,
    )

    # Generate the Go library.
    go_write_file = "%s.go.write_file" % name
    go_source = "%s.go" % name
    go_name = "%s_go" % name
    write_file(
        name = go_write_file,
        content = "\n".join(go_lines),
        out = go_source,
    )
    native.go_library(
        name = go_name,
        srcs = [go_source],
        visibility = visibility,
    )

    # Generate the Python library.
    py_write_file = "%s.py.write_file" % name
    py_source = "%s.py" % name
    py_name = "%s_py" % name
    write_file(
        name = py_write_file,
        content = "\n".join(py_lines),
        out = py_source,
    )
    native.py_library(
        name = py_name,
        srcs = [py_source],
        visibility = visibility,
        srcs_version = "PY3",
    )

    # Define a filegroup with all of the *_library rules. This is primarily
    # useful as a way to build all of the bundled libraries via a target with
    # the name of ${name}. Actual code needs to depend on its language-specific
    # library, not this.
    all_rules = [cc_name, go_name, py_name]
    native.filegroup(
        name = name,
        srcs = [":" + rule for rule in all_rules],
    )
