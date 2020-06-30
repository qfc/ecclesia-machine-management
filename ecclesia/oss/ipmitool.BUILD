licenses(["notice"])  # BSD

exports_files(["LICENSE"])

cc_library(
    name = "ipmi",
    srcs = [
        "lib/dimm_spd.c",
        "lib/helper.c",
        "lib/hpm2.c",
        "lib/ipmi_cfgp.c",
        "lib/ipmi_channel.c",
        "lib/ipmi_chassis.c",
        "lib/ipmi_dcmi.c",
        "lib/ipmi_delloem.c",
        "lib/ipmi_ekanalyzer.c",
        "lib/ipmi_event.c",
        "lib/ipmi_firewall.c",
        "lib/ipmi_fru.c",
        "lib/ipmi_fwum.c",
        "lib/ipmi_gendev.c",
        "lib/ipmi_hpmfwupg.c",
        "lib/ipmi_ime.c",
        "lib/ipmi_isol.c",
        "lib/ipmi_kontronoem.c",
        "lib/ipmi_lanp.c",
        "lib/ipmi_lanp6.c",
        "lib/ipmi_mc.c",
        "lib/ipmi_oem.c",
        "lib/ipmi_pef.c",
        "lib/ipmi_picmg.c",
        "lib/ipmi_raw.c",
        "lib/ipmi_sdr.c",
        "lib/ipmi_sdradd.c",
        "lib/ipmi_sel.c",
        "lib/ipmi_sensor.c",
        "lib/ipmi_session.c",
        "lib/ipmi_sol.c",
        "lib/ipmi_strings.c",
        "lib/ipmi_sunoem.c",
        "lib/ipmi_tsol.c",
        "lib/ipmi_user.c",
        "lib/ipmi_vita.c",
        "lib/log.c",
        "src/plugins/imb/imb.c",
        "src/plugins/imb/imbapi.c",
        "src/plugins/ipmi_intf.c",
        "src/plugins/lan/auth.c",
        "src/plugins/lan/lan.c",
        "src/plugins/lan/md5.c",
        "src/plugins/lanplus/lanplus.c",
        "src/plugins/lanplus/lanplus_crypt.c",
        "src/plugins/lanplus/lanplus_crypt_impl.c",
        "src/plugins/lanplus/lanplus_dump.c",
        "src/plugins/lanplus/lanplus_strings.c",
        "src/plugins/open/open.c",
        "src/plugins/serial/serial_basic.c",
        "src/plugins/serial/serial_terminal.c",
    ],
    hdrs = [
        "include/config.h",
        "include/ipmitool/bswap.h",
        "include/ipmitool/helper.h",
        "include/ipmitool/hpm2.h",
        "include/ipmitool/ipmi.h",
        "include/ipmitool/ipmi_cc.h",
        "include/ipmitool/ipmi_cfgp.h",
        "include/ipmitool/ipmi_channel.h",
        "include/ipmitool/ipmi_chassis.h",
        "include/ipmitool/ipmi_constants.h",
        "include/ipmitool/ipmi_dcmi.h",
        "include/ipmitool/ipmi_delloem.h",
        "include/ipmitool/ipmi_ekanalyzer.h",
        "include/ipmitool/ipmi_entity.h",
        "include/ipmitool/ipmi_event.h",
        "include/ipmitool/ipmi_firewall.h",
        "include/ipmitool/ipmi_fru.h",
        "include/ipmitool/ipmi_fwum.h",
        "include/ipmitool/ipmi_gendev.h",
        "include/ipmitool/ipmi_hpmfwupg.h",
        "include/ipmitool/ipmi_ime.h",
        "include/ipmitool/ipmi_intf.h",
        "include/ipmitool/ipmi_isol.h",
        "include/ipmitool/ipmi_kontronoem.h",
        "include/ipmitool/ipmi_lanp.h",
        "include/ipmitool/ipmi_lanp6.h",
        "include/ipmitool/ipmi_mc.h",
        "include/ipmitool/ipmi_oem.h",
        "include/ipmitool/ipmi_pef.h",
        "include/ipmitool/ipmi_picmg.h",
        "include/ipmitool/ipmi_raw.h",
        "include/ipmitool/ipmi_sdr.h",
        "include/ipmitool/ipmi_sdradd.h",
        "include/ipmitool/ipmi_sel.h",
        "include/ipmitool/ipmi_sel_supermicro.h",
        "include/ipmitool/ipmi_sensor.h",
        "include/ipmitool/ipmi_session.h",
        "include/ipmitool/ipmi_sol.h",
        "include/ipmitool/ipmi_strings.h",
        "include/ipmitool/ipmi_sunoem.h",
        "include/ipmitool/ipmi_tsol.h",
        "include/ipmitool/ipmi_user.h",
        "include/ipmitool/ipmi_vita.h",
        "include/ipmitool/log.h",
        "src/plugins/imb/imbapi.h",
        "src/plugins/lan/asf.h",
        "src/plugins/lan/auth.h",
        "src/plugins/lan/lan.h",
        "src/plugins/lan/md5.h",
        "src/plugins/lan/rmcp.h",
        "src/plugins/lanplus/asf.h",
        "src/plugins/lanplus/lanplus.h",
        "src/plugins/lanplus/lanplus_crypt.h",
        "src/plugins/lanplus/lanplus_crypt_impl.h",
        "src/plugins/lanplus/lanplus_dump.h",
        "src/plugins/lanplus/rmcp.h",
        "src/plugins/open/open.h",
    ],
    copts = [
        "-w",
        "-DHAVE_CONFIG_H",
    ],
    includes = [
        "include",
    ],
    linkopts = [
        "-lm",
    ],
    deps = [
        "@libedit//:pretend_to_be_gnu_readline_system",
        "@ncurses//:ncurses",
        "@boringssl//:crypto",
    ],
)
