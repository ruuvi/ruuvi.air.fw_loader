/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "fwloader_button_cb.h"
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/atomic_builtin.h>
#include <zephyr/retention/bootmode.h>
#include <zephyr/logging/log.h>
#include "fwloader_button.h"
#include "fwloader_led.h"
#include "fwloader_watchdog.h"

LOG_MODULE_DECLARE(fw_loader, LOG_LEVEL_INF);

#define RUUVI_AIR_BUTTON_DELAY_FLUSH_LOGS_MS (100)

static struct gpio_callback g_button_isr_gpio_cb_data;

static void
button_workq_cb_pressed(struct k_work* item);
static void
button_workq_cb_released(struct k_work* item);
static void
button_workq_cb_timeout(struct k_work* item);
static void
button_workq_cb_reboot(struct k_work* item);

static K_WORK_DEFINE(g_button_work_pressed, &button_workq_cb_pressed);
static K_WORK_DEFINE(g_button_work_released, &button_workq_cb_released);
static K_WORK_DELAYABLE_DEFINE(g_button_work_delayable_timeout, &button_workq_cb_timeout);
static K_WORK_DELAYABLE_DEFINE(g_button_work_delayable_reboot, &button_workq_cb_reboot);

static void
button_workq_cb_pressed(__unused struct k_work* item)
{
    k_work_reschedule(&g_button_work_delayable_timeout, K_MSEC(CONFIG_RUUVI_AIR_BUTTON_DELAY_BEFORE_REBOOT));
    fwloader_led_lock();
    fwloader_button_set_pressed();
    fwloader_led_green_on();
    fwloader_led_red_on();
    fwloader_led_unlock();
    LOG_WRN("Button pressed");
}

static void
button_workq_cb_released(__unused struct k_work* item)
{
    k_work_cancel_delayable(&g_button_work_delayable_timeout);
    LOG_WRN("Button released - rebooting...");
    fwloader_led_lock();
    fwloader_led_green_off();
    fwloader_led_red_off();
    fwloader_led_unlock();
    k_work_reschedule(&g_button_work_delayable_reboot, K_MSEC(RUUVI_AIR_BUTTON_DELAY_FLUSH_LOGS_MS));
}

static void
button_workq_cb_timeout(__unused struct k_work* item)
{
    LOG_WRN("Button %d ms timeout - rebooting...", CONFIG_RUUVI_AIR_BUTTON_DELAY_BEFORE_REBOOT);
    fwloader_led_lock();
    fwloader_led_green_off();
    fwloader_led_red_off();
    fwloader_led_unlock();
    k_work_reschedule(&g_button_work_delayable_reboot, K_MSEC(RUUVI_AIR_BUTTON_DELAY_FLUSH_LOGS_MS));
}

static void
button_workq_cb_reboot(__unused struct k_work* item)
{
    sys_reboot(SYS_REBOOT_COLD);
}

static void
fwloader_isr_cb_pinhole_button_pressed_or_released(
    __unused const struct device*  dev,
    __unused struct gpio_callback* cb,
    __unused uint32_t              pins)
{
    if (fwloader_button_get())
    {
        k_work_submit(&g_button_work_pressed);
    }
    else
    {
        k_work_submit(&g_button_work_released);
    }
}

void
fwloader_button_cb_init(void)
{
    fwloader_button_init(
        &g_button_isr_gpio_cb_data,
        &fwloader_isr_cb_pinhole_button_pressed_or_released,
        GPIO_INT_EDGE_BOTH);
}

void
fwloader_button_cb_deinit(void)
{
    fwloader_button_deinit(&g_button_isr_gpio_cb_data);
}
