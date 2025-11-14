/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "fwloader_watchdog.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wdog, LOG_LEVEL_INF);

#if IS_ENABLED(CONFIG_WATCHDOG) && DT_NODE_EXISTS(DT_ALIAS(watchdog0)) && DT_NODE_HAS_STATUS(DT_ALIAS(watchdog0), okay)
static const struct device* g_wdt_dev = DEVICE_DT_GET(DT_ALIAS(watchdog0));
static int                  g_wdt_channel;
#endif

bool
fwloader_watchdog_start(void)
{
#if !CONFIG_DEBUG && IS_ENABLED(CONFIG_WATCHDOG)
#if DT_NODE_EXISTS(DT_ALIAS(watchdog0)) && DT_NODE_HAS_STATUS(DT_ALIAS(watchdog0), okay)
    if (!device_is_ready(g_wdt_dev))
    {
        LOG_ERR("watchdog device not ready");
        return false;
    }

    struct wdt_timeout_cfg cfg = {
		.window = {
			.min = 0,
			.max = CONFIG_RUUVI_AIR_BUTTON_DELAY_BEFORE_REBOOT,
		},
		.callback = NULL,
		.flags = WDT_FLAG_RESET_SOC,
	};

    g_wdt_channel = wdt_install_timeout(g_wdt_dev, &cfg);
    if (g_wdt_channel < 0)
    {
        LOG_ERR("wdt_install_timeout failed: %d", g_wdt_channel);
        return false;
    }

    int err = wdt_setup(g_wdt_dev, 0);
    if (0 != err)
    {
        LOG_ERR("wdt_setup failed: %d", err);
        return false;
    }

    LOG_INF("WDT started: %d ms timeout", CONFIG_RUUVI_AIR_BUTTON_DELAY_BEFORE_REBOOT);
#endif
#endif // !CONFIG_DEBUG && IS_ENABLED(CONFIG_WATCHDOG)
    return true;
}

void
fwloader_watchdog_feed(void)
{
#if !CONFIG_DEBUG && IS_ENABLED(CONFIG_WATCHDOG)
#if DT_NODE_EXISTS(DT_ALIAS(watchdog0)) && DT_NODE_HAS_STATUS(DT_ALIAS(watchdog0), okay)
    int err = wdt_feed(g_wdt_dev, g_wdt_channel);
    if (err)
    {
        LOG_ERR("wdt_feed failed: %d", err);
    }
    else
    {
        LOG_DBG("wdt fed");
    }
#endif
#endif // !CONFIG_DEBUG && IS_ENABLED(CONFIG_WATCHDOG)
}

#if !CONFIG_DEBUG && IS_ENABLED(CONFIG_WATCHDOG)
__NO_RETURN void
fwloader_watchdog_force_trigger(void)
{
    (void)irq_lock();

    __DSB();
    __ISB();

    for (;;)
    {
        __NOP();
    }
}
#endif // !CONFIG_DEBUG && IS_ENABLED(CONFIG_WATCHDOG)
