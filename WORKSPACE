workspace(name = "com_google_ecclesia")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Google Test. Official release 1.10.0.
http_archive(
    name = "com_google_googletest",
    sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
    strip_prefix = "googletest-release-1.10.0",
    urls = ["https://github.com/google/googletest/archive/release-1.10.0.tar.gz"],
)

# Abseil. Latest feature not releases yet. Picked up a commit from Sep 2, 2020
http_archive(
    name = "com_google_absl",
    sha256 = "fc34c6d71993827eec8e77675086563a378b24ed8072a52b50804f2c29f19709",
    strip_prefix = "abseil-cpp-930fbec75b452af8bb8c796f5bb754e953e29cf5",
    urls = ["https://github.com/abseil/abseil-cpp/archive/930fbec75b452af8bb8c796f5bb754e953e29cf5.zip"],
)

# emboss. No official releases yet. Picked up a commit from Oct 9, 2019
http_archive(
    name = "com_google_emboss",
    sha256 = "e4f6984b7177481f3b6b9c3457a2f7a7662b72aa79da5c421924e7b78c124172",
    strip_prefix = "emboss-26fac66f7e295145715306a26068a9c8dc334f7b",
    urls = ["https://github.com/google/emboss/archive/26fac66f7e295145715306a26068a9c8dc334f7b.zip"],
)

# Protocol buffers. Official release 3.11.4.
http_archive(
    name = "com_google_protobuf",
    sha256 = "a79d19dcdf9139fa4b81206e318e33d245c4c9da1ffed21c87288ed4380426f9",
    strip_prefix = "protobuf-3.11.4",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.11.4.tar.gz"],
)

http_archive(
    name = "rules_python",
    sha256 = "e5470e92a18aa51830db99a4d9c492cc613761d5bdb7131c04bd92b9834380f6",
    strip_prefix = "rules_python-4b84ad270387a7c439ebdccfd530e2339601ef27",
    urls = ["https://github.com/bazelbuild/rules_python/archive/4b84ad270387a7c439ebdccfd530e2339601ef27.tar.gz"],
)

http_archive(
    name = "bazel_skylib",
    sha256 = "ea5dc9a1d51b861d27608ad3bd6c177bc88d54e946cb883e9163e53c607a9b4c",
    strip_prefix = "bazel-skylib-2b38b2f8bd4b8603d610cfc651fcbb299498147f",
    urls = ["https://github.com/bazelbuild/bazel-skylib/archive/2b38b2f8bd4b8603d610cfc651fcbb299498147f.tar.gz"],
)

http_archive(
    name = "rules_pkg",
    sha256 = "b9d1387deed06eef45edd3eb7fd166577b8ad1884cb6a17898d136059d03933c",
    strip_prefix = "rules_pkg-0.2.6-1/pkg",
    urls = ["https://github.com/bazelbuild/rules_pkg/archive/0.2.6-1.tar.gz"],
)

http_archive(
    name = "subpar",
    sha256 = "b80297a1b8d38027a86836dbadc22f55dc3ecad56728175381aa6330705ac10f",
    strip_prefix = "subpar-2.0.0",
    urls = ["https://github.com/google/subpar/archive/2.0.0.tar.gz"],
)

# TensorFlow depends on "io_bazel_rules_closure" so we need this here.
# Needs to be kept in sync with the same target in TensorFlow's WORKSPACE file.
http_archive(
    name = "io_bazel_rules_closure",
    sha256 = "9b99615f73aa574a2947226c6034a6f7c319e1e42905abc4dc30ddbbb16f4a31",
    strip_prefix = "rules_closure-4a79cc6e6eea5e272fe615db7c98beb8cf8e7eb5",
    urls = [
        "http://mirror.tensorflow.org/github.com/bazelbuild/rules_closure/archive/4a79cc6e6eea5e272fe615db7c98beb8cf8e7eb5.tar.gz",
        "https://github.com/bazelbuild/rules_closure/archive/4a79cc6e6eea5e272fe615db7c98beb8cf8e7eb5.tar.gz",  # 2019-09-16
    ],
)

http_archive(
    name = "com_github_libevent_libevent",
    build_file = "@//ecclesia/oss:libevent.BUILD",
    sha256 = "70158101eab7ed44fd9cc34e7f247b3cae91a8e4490745d9d6eb7edc184e4d96",
    strip_prefix = "libevent-release-2.1.8-stable",
    urls = [
        "https://github.com/libevent/libevent/archive/release-2.1.8-stable.zip",
    ],
)

http_archive(
    name = "com_googlesource_code_re2",
    sha256 = "d070e2ffc5476c496a6a872a6f246bfddce8e7797d6ba605a7c8d72866743bf9",
    strip_prefix = "re2-506cfa4bffd060c06ec338ce50ea3468daa6c814",
    urls = [
        "https://github.com/google/re2/archive/506cfa4bffd060c06ec338ce50ea3468daa6c814.tar.gz",
    ],
)

http_archive(
    name = "org_tensorflow",
    # NOTE: when updating this, MAKE SURE to also update the protobuf_js runtime version
    # in third_party/workspace.bzl to >= the protobuf/protoc version provided by TF.
    sha256 = "48ddba718da76df56fd4c48b4bbf4f97f254ba269ec4be67f783684c75563ef8",
    strip_prefix = "tensorflow-2.0.0-rc0",
    urls = [
        "http://mirror.tensorflow.org/github.com/tensorflow/tensorflow/archive/v2.0.0-rc0.tar.gz",  # 2019-08-23
        "https://github.com/tensorflow/tensorflow/archive/v2.0.0-rc0.tar.gz",
    ],
)

#tensoflow. Official release 2.0.0 from Oct 18, 2019
http_archive(
    name = "com_google_tensorflow_serving",
    sha256 = "70f0847bb26bac948719d75b4760443cee88f4aa4e208c2bef1d6a6baad708a0",
    strip_prefix = "serving-b5a11f1e5388c9985a6fc56a58c3421e5f78149f",
    urls = ["https://github.com/tensorflow/serving/archive/b5a11f1e5388c9985a6fc56a58c3421e5f78149f.zip"],
)

load("@com_google_tensorflow_serving//tensorflow_serving:workspace.bzl", "tf_serving_workspace")

tf_serving_workspace()

#jsoncpp. Official release 1.9.2.
http_archive(
    name = "com_jsoncpp",
    build_file = "@//ecclesia/oss:jsoncpp.BUILD",
    sha256 = "77a402fb577b2e0e5d0bdc1cf9c65278915cdb25171e3452c68b6da8a561f8f0",
    strip_prefix = "jsoncpp-1.9.2",
    urls = ["https://github.com/open-source-parsers/jsoncpp/archive/1.9.2.tar.gz"],
)

http_archive(
    name = "zlib",
    build_file = "@com_google_protobuf//:third_party/zlib.BUILD",
    sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
    strip_prefix = "zlib-1.2.11",
    urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],
)

http_archive(
    name = "boringssl",
    sha256 = "66e1b0675d58b35f9fe3224b26381a6d707c3293eeee359c813b4859a6446714",
    strip_prefix = "boringssl-9b7498d5aba71e545747d65dc65a4d4424477ff0",
    urls = [
        "https://github.com/google/boringssl/archive/9b7498d5aba71e545747d65dc65a4d4424477ff0.tar.gz",
    ],
)

http_archive(
    name = "ncurses",
    build_file = "@//ecclesia/oss:ncurses.BUILD",
    sha256 = "aa057eeeb4a14d470101eff4597d5833dcef5965331be3528c08d99cebaa0d17",
    strip_prefix = "ncurses-6.1",
    urls = [
        "http://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.1.tar.gz"
    ],
)

http_archive(
    name = "libedit",
    build_file = "@//ecclesia/oss:libedit.BUILD",
    sha256 = "dbb82cb7e116a5f8025d35ef5b4f7d4a3cdd0a3909a146a39112095a2d229071",
    strip_prefix = "libedit-20191231-3.1",
    urls = [
        "https://www.thrysoee.dk/editline/libedit-20191231-3.1.tar.gz",
    ],
)

http_archive(
    name = "ipmitool",
    build_file = "@//ecclesia/oss:ipmitool.BUILD",
    sha256 = "e93fe5966d59e16bb4317c9c588cdf35d6100753a0ba957c493b064bcad52493",
    strip_prefix = "ipmitool-1.8.18",
    urls = [
        "https://github.com/ipmitool/ipmitool/releases/download/IPMITOOL_1_8_18/ipmitool-1.8.18.tar.gz"
    ],
    patches = [
        # Openssl 1.1 made struct EVP_MD_CTX opaque, so we have to heap
        # allocate it.
        "//ecclesia/oss:ipmitool.lanplus_crypt_impl.patch"
    ],
    patch_cmds = [
        "./configure CFLAGS=-fPIC CXXFLAGS=-fPIC --enable-shared=no",
        "cp ./config.h include",
    ],
)

http_archive(
    name = "curl",
    build_file = "@//ecclesia/oss:curl.BUILD",
    sha256 = "01ae0c123dee45b01bbaef94c0bc00ed2aec89cb2ee0fd598e0d302a6b5e0a98",
    strip_prefix = "curl-7.69.1",
    urls = [
        "https://storage.googleapis.com/mirror.tensorflow.org/curl.haxx.se/download/curl-7.69.1.tar.gz",
        "https://curl.haxx.se/download/curl-7.69.1.tar.gz",
    ],
)

http_archive(
    name = "jansson",
    build_file = "@//ecclesia/oss:jansson.BUILD",
    sha256 = "5f8dec765048efac5d919aded51b26a32a05397ea207aa769ff6b53c7027d2c9",
    strip_prefix = "jansson-2.12",
    urls = [
        "https://digip.org/jansson/releases/jansson-2.12.tar.gz",
    ],
)

http_archive(
    name = "libredfish",
    build_file = "@//ecclesia/oss:libredfish.BUILD",
    sha256 = "301563b061da5862e2dfa7da367d37298856eace5aabba80cabf15a42b6ed3d3",
    strip_prefix = "libredfish-1.2.8",
    urls = [
        "https://github.com/DMTF/libredfish/archive/1.2.8.tar.gz",
    ],
    patches = [
        "//ecclesia/oss:01.redfishService.h.patch",
        "//ecclesia/oss:02.queue.h.patch",
        "//ecclesia/oss:03.queue.c.patch",
        "//ecclesia/oss:04.service.c.patch",
        "//ecclesia/oss:05.internal_service.h.patch",
        "//ecclesia/oss:06.asyncRaw.c.patch",
    ],
)

http_archive(
    name = "redfishMockupServer",
    build_file = "@//ecclesia/oss:redfishMockupServer.BUILD",
    sha256 = "2a4663441b205189686dfcc9a4082fc8c30cb1637126281fb291ad69cabb43f9",
    strip_prefix = "Redfish-Mockup-Server-1.1.0",
    urls = ["https://github.com/DMTF/Redfish-Mockup-Server/archive/1.1.0.tar.gz"],
    patches = [
        "//ecclesia/oss:redfishMockupServer.patches/01.remove_grequest.patch",
        "//ecclesia/oss:redfishMockupServer.patches/02.add_ipv6.patch",
        "//ecclesia/oss:redfishMockupServer.patches/03.fix_traversal_vulnerability.patch",
        "//ecclesia/oss:redfishMockupServer.patches/04.add_uds.patch",
    ],
)
