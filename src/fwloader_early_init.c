/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <pm_config.h>
#include <fprotect.h>
#include "fwloader_supercap.h"
#include "fwloader_led.h"
#include "fwloader_button_cb.h"
#include "fwloader_ext_flash_power.h"
#include "zephyr_api.h"

LOG_MODULE_DECLARE(fw_loader, LOG_LEVEL_INF);

#define CONFIG_RUUVI_AIR_GPIO_EXT_FLASH_POWER_ON_PRIORITY 41
_Static_assert(CONFIG_RUUVI_AIR_GPIO_EXT_FLASH_POWER_ON_PRIORITY > CONFIG_GPIO_INIT_PRIORITY);
_Static_assert(CONFIG_RUUVI_AIR_GPIO_EXT_FLASH_POWER_ON_PRIORITY < CONFIG_NORDIC_QSPI_NOR_INIT_PRIORITY);

#define EARLY_INIT_PERIPHERAL_POWER_OFF_DELAY_MS (100)
#define EARLY_INIT_PERIPHERAL_POWER_ON_DELAY_MS  (100)

static int // NOSONAR: Zephyr init functions must return int
fwloader_early_init_post_kernel(void)
{
    printk("\r\n*** %s ***\r\n", CONFIG_NCS_APPLICATION_BOOT_BANNER_STRING);
#if defined(CONFIG_BOARD_RUUVI_RUUVIAIR_REV_1)
    fwloader_supercap_init();
#endif // CONFIG_BOARD_RUUVI_RUUVIAIR_REV_1
    fwloader_led_early_init();
    fwloader_ext_flash_power_off();
    fwloader_led_red_set(true);
    k_msleep(EARLY_INIT_PERIPHERAL_POWER_OFF_DELAY_MS);
    fwloader_button_cb_init();
    fwloader_ext_flash_power_on();
    fwloader_led_red_set(false);
    k_msleep(EARLY_INIT_PERIPHERAL_POWER_ON_DELAY_MS);
    return 0;
}

SYS_INIT(fwloader_early_init_post_kernel, POST_KERNEL, CONFIG_RUUVI_AIR_GPIO_EXT_FLASH_POWER_ON_PRIORITY);

#if defined(PM_MCUBOOT_SECONDARY_ADDRESS)
static int // NOSONAR: Zephyr init functions must return int
fprotect_self(void)
{
    LOG_INF(
        "Protecting app area: address 0x%08" PRIx32 ", size %" PRIx32,
        PM_MCUBOOT_SECONDARY_ADDRESS,
        PM_MCUBOOT_SECONDARY_SIZE);
    zephyr_api_ret_t err = fprotect_area(PM_MCUBOOT_SECONDARY_ADDRESS, PM_MCUBOOT_SECONDARY_SIZE);
    if (err != 0)
    {
        __ASSERT(
            0,
            "Unable to lock required area. Check address and "
            "size against locking granularity.");
    }
    return 0;
}

SYS_INIT(fprotect_self, APPLICATION, 0);
#endif // PM_MCUBOOT_SECONDARY_ADDRESS
