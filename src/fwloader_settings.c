/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "fwloader_settings.h"
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>
#include "fwloader_fw_ver.h"
#include "app_version.h"

LOG_MODULE_DECLARE(fw_loader, LOG_LEVEL_INF);

static void
settings_runtime_load(void)
{
#if defined(CONFIG_BT_DIS_SETTINGS)
    settings_runtime_set("bt/dis/model", CONFIG_BT_DIS_MODEL, sizeof(CONFIG_BT_DIS_MODEL));
    settings_runtime_set("bt/dis/manuf", CONFIG_BT_DIS_MANUF, sizeof(CONFIG_BT_DIS_MANUF));
#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
    settings_runtime_set("bt/dis/serial", CONFIG_BT_DIS_SERIAL_NUMBER_STR, sizeof(CONFIG_BT_DIS_SERIAL_NUMBER_STR));
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
    settings_runtime_set("bt/dis/sw", CONFIG_BT_DIS_SW_REV_STR, sizeof(CONFIG_BT_DIS_SW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
    char fw_ver_str[sizeof(CONFIG_BT_DEVICE_NAME) + 2 + sizeof(APP_VERSION_EXTENDED_STRING)];
    snprintf(fw_ver_str, sizeof(fw_ver_str), "%s v%s", CONFIG_BT_DEVICE_NAME, fwloader_fw_ver_get());
    settings_runtime_set("bt/dis/fw", fw_ver_str, strlen(fw_ver_str) + 1);
#endif
    const char* const p_hw_rev_str = fwloader_hw_rev_get();
    settings_runtime_set("bt/dis/hw", p_hw_rev_str, strlen(p_hw_rev_str) + 1);
#endif
}

bool
fwloader_settings_init(void)
{
    settings_runtime_load();
    return true;
}
