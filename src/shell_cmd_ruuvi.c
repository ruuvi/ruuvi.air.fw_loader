/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include "fwloader_settings.h"
#include "fw_img_hw_rev.h"
#include "fwloader_fw_ver.h"

LOG_MODULE_REGISTER(shell_cmd_ruuvi, LOG_LEVEL_INF);

static void
log_args(size_t argc, char** argv)
{
    LOG_DBG("%s: argc=%zu", __func__, argc);
    for (size_t i = 0; i < argc; i++)
    {
        LOG_DBG("%s: argv[%zu]=%s", __func__, i, argv[i]);
    }
}

static int
cmd_ruuvi_echo(const struct shell* sh, size_t argc, char** argv)
{
    log_args(argc, argv);

    const char* const p_message = argv[1];
    LOG_DBG("Echo: %s", p_message);

    shell_print(sh, "%s", p_message);

    return 0;
}

#if defined(CONFIG_BOOTLOADER_MCUBOOT)
static int
cmd_ruuvi_version_info(const struct shell* sh, size_t argc, char** argv)
{
    log_args(argc, argv);

    const char* const p_version_str   = fwloader_fw_ver_get();
    const size_t      version_str_len = strlen(p_version_str);
    const char* const p_prod_suffix   = "-prod";
    const bool        is_prod         = (version_str_len >= strlen(p_prod_suffix))
                         && (0 == strcmp(&p_version_str[version_str_len - strlen(p_prod_suffix)], p_prod_suffix));

    struct image_version  fw_ver    = { 0 };
    const struct fw_info* p_fw_info = NULL;
    fw_image_hw_rev_t     hw_rev    = { 0 };
    if (!fw_img_get_image_info(FW_IMG_ID_APP, &fw_ver, &p_fw_info, &hw_rev))
    {
        shell_error(sh, "Failed to get firmware image info");
        return -EINVAL;
    }

    shell_print(sh, "Hardware revision: %s", hw_rev.hw_rev_name);
    shell_print(sh, "Build type: %s", is_prod ? "production" : "development");

    shell_print(
        sh,
        "App version: %u.%u.%u+%u",
        fw_ver.iv_major,
        fw_ver.iv_minor,
        fw_ver.iv_revision,
        fw_ver.iv_build_num);

    if (!fw_img_get_image_info(FW_IMG_ID_FWLOADER, &fw_ver, &p_fw_info, &hw_rev))
    {
        shell_error(sh, "Failed to get firmware image info");
        return -EINVAL;
    }
    shell_print(
        sh,
        "FwLoader version: %u.%u.%u+%u",
        fw_ver.iv_major,
        fw_ver.iv_minor,
        fw_ver.iv_revision,
        fw_ver.iv_build_num);

    if (!fw_img_get_image_info(FW_IMG_ID_MCUBOOT0, &fw_ver, &p_fw_info, &hw_rev))
    {
        shell_error(sh, "Failed to get firmware image info");
        return -EINVAL;
    }
    shell_print(
        sh,
        "MCUBoot0 version: %u.%u.%u+%u",
        fw_ver.iv_major,
        fw_ver.iv_minor,
        fw_ver.iv_revision,
        fw_ver.iv_build_num);

    if (!fw_img_get_image_info(FW_IMG_ID_MCUBOOT1, &fw_ver, &p_fw_info, &hw_rev))
    {
        shell_error(sh, "Failed to get firmware image info");
        return -EINVAL;
    }
    shell_print(
        sh,
        "MCUBoot1 version: %u.%u.%u+%u",
        fw_ver.iv_major,
        fw_ver.iv_minor,
        fw_ver.iv_revision,
        fw_ver.iv_build_num);

    return 0;
}
#endif // CONFIG_BOOTLOADER_MCUBOOT

/* Add command to the set of 'ruuvi' subcommands, see `SHELL_SUBCMD_ADD` */
#define RUUVI_CMD_ARG_ADD(_syntax, _subcmd, _help, _handler, _mand, _opt) \
    SHELL_SUBCMD_ADD((ruuvi), _syntax, _subcmd, _help, _handler, _mand, _opt);

RUUVI_CMD_ARG_ADD(echo, NULL, "message", cmd_ruuvi_echo, 2, 0);

#if defined(CONFIG_BOOTLOADER_MCUBOOT)
RUUVI_CMD_ARG_ADD(version_info, NULL, "version_info", cmd_ruuvi_version_info, 1, 0);
#endif // CONFIG_BOOTLOADER_MCUBOOT

SHELL_SUBCMD_SET_CREATE(ruuvi_cmds, (ruuvi));
SHELL_CMD_REGISTER(ruuvi, &ruuvi_cmds, "Ruuvi commands", NULL);
