/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "fwloader_led.h"
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(fw_loader, LOG_LEVEL_INF);

#if DT_NODE_EXISTS(DT_ALIAS(led_red))
#define LED_RED_NODE DT_ALIAS(led_red)
#else
#error "'led-red' devicetree alias is not defined"
#endif

#if DT_NODE_HAS_STATUS(LED_RED_NODE, okay) && DT_NODE_HAS_PROP(LED_RED_NODE, gpios)
static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(LED_RED_NODE, gpios);
#else
#error "'led-red' devicetree alias is not defined properly"
#endif

#if DT_NODE_EXISTS(DT_ALIAS(led_green))
#define LED_GREEN_NODE DT_ALIAS(led_green)
#else
#error "'led-green' devicetree alias is not defined"
#endif
#if DT_NODE_HAS_STATUS(LED_GREEN_NODE, okay) && DT_NODE_HAS_PROP(LED_GREEN_NODE, gpios)
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(LED_GREEN_NODE, gpios);
#else
#error "'led-green' devicetree alias is not defined properly"
#endif

static K_MUTEX_DEFINE(g_led_mutex);

static void
fwloader_led_init_gpio(const struct gpio_dt_spec* p_led_spec)
{
    if (!device_is_ready(p_led_spec->port))
    {
        LOG_ERR("LED %s:%d is not ready", p_led_spec->port->name, p_led_spec->pin);
        return;
    }

    const int rc = gpio_pin_configure_dt(p_led_spec, GPIO_OUTPUT_INACTIVE);
    if (0 != rc)
    {
        LOG_ERR("Failed to configure LED %s:%d, rc %d", p_led_spec->port->name, p_led_spec->pin, rc);
        return;
    }
}

void
fwloader_led_init(void)
{
    fwloader_led_init_gpio(&led_red);
    fwloader_led_init_gpio(&led_green);
}

static void
fwloader_led_deinit_gpio(const struct gpio_dt_spec* p_led_spec)
{
    if (!device_is_ready(p_led_spec->port))
    {
        LOG_ERR("LED %s:%d is not ready", p_led_spec->port->name, p_led_spec->pin);
        return;
    }

    gpio_pin_set_dt(p_led_spec, 0);

    const int rc = gpio_pin_configure_dt(p_led_spec, GPIO_DISCONNECTED);
    if (0 != rc)
    {
        LOG_ERR("Failed to configure LED %s:%d, rc %d", p_led_spec->port->name, p_led_spec->pin, rc);
        return;
    }
}

void
fwloader_led_deinit(void)
{
    fwloader_led_deinit_gpio(&led_red);
    fwloader_led_deinit_gpio(&led_green);
}

void
fwloader_led_red_set(const bool is_on)
{
    gpio_pin_set_dt(&led_red, is_on ? 1 : 0);
}

void
fwloader_led_green_set(const bool is_on)
{
    gpio_pin_set_dt(&led_green, is_on ? 1 : 0);
}

void
fwloader_led_lock(void)
{
    k_mutex_lock(&g_led_mutex, K_FOREVER);
}

void
fwloader_led_unlock(void)
{
    k_mutex_unlock(&g_led_mutex);
}
