licenses(["notice"])

exports_files(["LICENSE"])

include_files = [
    "libcurl/include/curl/curl.h",
    "libcurl/include/curl/curlver.h",
    "libcurl/include/curl/easy.h",
    "libcurl/include/curl/mprintf.h",
    "libcurl/include/curl/multi.h",
    "libcurl/include/curl/stdcheaders.h",
    "libcurl/include/curl/system.h",
    "libcurl/include/curl/typecheck-gcc.h",
    "libcurl/include/curl/urlapi.h",
]

lib_files = [
    "libcurl/lib/libcurl.a",
]

genrule(
    name = "curl_srcs",
    srcs = glob(["**"]),
    outs = include_files + lib_files,
    cmd = "\n".join([
        "export INSTALL_DIR=$$(pwd)/$(@D)/libcurl",
        "export TMP_DIR=$$(mktemp -d -t libcurl.XXXXXX)",
        "mkdir -p $$TMP_DIR",
        "cp -R $$(pwd)/external/curl/* $$TMP_DIR",
        "cd $$TMP_DIR",
        "./configure --prefix=$$INSTALL_DIR CFLAGS=-fPIC CXXFLAGS=-fPIC --without-ssl --without-librtmp --disable-ldap --disable-ldaps",
        "make install",
        "rm -rf $$TMP_DIR",
    ]),
)

cc_library(
    name = "curl",
    srcs = ["libcurl/lib/libcurl.a"],
    hdrs = include_files,
    includes = ["libcurl/include"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = ["@zlib"],
)
