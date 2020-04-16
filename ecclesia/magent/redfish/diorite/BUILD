licenses(["notice"])

cc_library(
    name = "diorite",
    hdrs = [
        "chassis.h",
        "power.h",
        "redfish_service.h",
        "root.h",
        "service_root.h",
        "system.h",
        "systems.h",
        "thermal.h",
    ],
    visibility = ["//:magent_frontend_users"],
    deps = [
        "@com_google_absl//absl/strings",
        "//magent/redfish/core:redfish_core",
        "//jsoncpp:json",
        "@com_google_tensorflow_serving//tensorflow_serving/util/net_http/server/public:http_server_api",
    ],
)
