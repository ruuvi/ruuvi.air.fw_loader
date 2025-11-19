/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "fwloader_led_err.h"
#include <stdint.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include "fwloader_led.h"
#include "fwloader_button.h"
#include "fwloader_button_cb.h"

LOG_MODULE_DECLARE(fw_loader, LOG_LEVEL_INF);

#define SLEEP_INTERVAL_MS       (10)
#define NUM_BLINKS_PREFIX       (2)
#define DELAY_BETWEEN_BLINKS_MS (100)
#define DELAY_AFTER_PREFIX_MS   (500)
#define DELAY_AFTER_ERR_CODE_MS (1000)

#define DELAY_BEFORE_REBOOT_MS (100)

static bool
check_if_button_released_and_pressed(bool* p_is_button_released)
{
    if (!*p_is_button_released)
    {
        if (!fwloader_button_get())
        {
            *p_is_button_released = true;
            LOG_INF("fw_loader: Button is released");
            LOG_INF("fw_loader: Wait until button is pressed to reboot");
        }
    }
    else
    {
        if (fwloader_button_get())
        {
            LOG_INF("fw_loader: Button is pressed - reboot");
            k_msleep(DELAY_BEFORE_REBOOT_MS);
            return true;
        }
    }
    return false;
}

static void
fwloader_led_err_blink(
    bool* const    p_is_button_released,
    const uint32_t num_blinks,
    const uint32_t delay_between_blinks_ms,
    const uint32_t delay_after_blinks_ms)
{
    for (uint32_t i = 0; i < num_blinks; ++i)
    {
        fwloader_led_red_on();

        for (uint32_t j = 0; j < (delay_between_blinks_ms / SLEEP_INTERVAL_MS); ++j)
        {
            k_msleep(SLEEP_INTERVAL_MS);
            if (check_if_button_released_and_pressed(p_is_button_released))
            {
                sys_reboot(SYS_REBOOT_COLD);
            }
        }

        fwloader_led_red_off();

        for (uint32_t j = 0; j < (delay_after_blinks_ms / SLEEP_INTERVAL_MS); ++j)
        {
            k_msleep(SLEEP_INTERVAL_MS);
            if (check_if_button_released_and_pressed(p_is_button_released))
            {
                sys_reboot(SYS_REBOOT_COLD);
            }
        }
    }
}

__NO_RETURN void
fwloader_led_err_blink_red_led(const uint32_t num_red_blinks)
{
    fwloader_button_cb_deinit();

    bool is_button_released = !fwloader_button_get();
    if (is_button_released)
    {
        LOG_INF("fw_loader: Wait until button is pressed to reboot");
    }
    fwloader_led_green_off();
    while (1)
    {
        fwloader_led_err_blink(&is_button_released, NUM_BLINKS_PREFIX, DELAY_BETWEEN_BLINKS_MS, DELAY_AFTER_PREFIX_MS);
        fwloader_led_err_blink(&is_button_released, num_red_blinks, DELAY_BETWEEN_BLINKS_MS, DELAY_AFTER_ERR_CODE_MS);
    }
}
