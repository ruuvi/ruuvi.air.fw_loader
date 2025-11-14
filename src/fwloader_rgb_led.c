/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "fwloader_rgb_led.h"
#include <stdint.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>
#include <lp5810_api.h>

LOG_MODULE_DECLARE(fw_loader, LOG_LEVEL_INF);

#define LED_RGB_MAX_BRIGHTNESS (255U)
#define LED_RGB_MAX_PWM        (255U)

#define LED_RGB_CHANNEL_CURRENT_START 0
#define LED_RGB_CHANNEL_PWM_START     3

#if DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
const struct device* const dev_lp5810 = DEVICE_DT_GET_ONE(ti_lp5810);
#endif // DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)

bool
fwloader_rgb_led_is_lp5810_ready(void)
{
#if DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
    return device_is_ready(dev_lp5810);
#else
    return false;
#endif // DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
}

bool
fwloader_rgb_led_set_raw_currents_and_pwms(
    const fwloader_rgb_led_currents_t* const p_rgb_led_currents,
    const fwloader_rgb_led_pwms_t* const     p_rgb_led_pwms)
{
#if DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
    uint8_t buf[6] = {
        p_rgb_led_currents->current_red, p_rgb_led_currents->current_green, p_rgb_led_currents->current_blue,
        p_rgb_led_pwms->pwm_red,         p_rgb_led_pwms->pwm_green,         p_rgb_led_pwms->pwm_blue,
    };
    LOG_HEXDUMP_DBG(buf, sizeof(buf), "RGB LED update: ");
    int res = led_write_channels(dev_lp5810, LED_RGB_CHANNEL_CURRENT_START, sizeof(buf), buf);
    if (0 != res)
    {
        LOG_ERR("LP5810: led_write_channels failed, res=%d", res);
        return false;
    }
    return true;
#else
    return true;
#endif // DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
}

bool
fwloader_rgb_led_update_pwms(const fwloader_rgb_led_pwms_t* const p_rgb_led_pwms)
{
#if DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
    uint8_t buf[3] = {
        p_rgb_led_pwms->pwm_red,
        p_rgb_led_pwms->pwm_green,
        p_rgb_led_pwms->pwm_blue,
    };
    LOG_HEXDUMP_DBG(buf, sizeof(buf), "RGB LED update: ");
    int res = led_write_channels(dev_lp5810, LED_RGB_CHANNEL_PWM_START, sizeof(buf), buf);
    if (0 != res)
    {
        LOG_ERR("LP5810: led_write_channels failed, res=%d", res);
        return false;
    }
    return true;
#else
    return true;
#endif // DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
}

bool
fwloader_rgb_led_check_and_reinit_if_needed(void)
{
#if DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
    if (!device_is_ready(dev_lp5810))
    {
        LOG_ERR("Device %s is not ready", dev_lp5810->name);
        return false;
    }
    return lp5810_check_and_reinit_if_needed(dev_lp5810);
#else
    return true;
#endif // DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
}

bool
fwloader_rgb_led_turn_on_animation_blinking_white(const fwloader_rgb_led_currents_t* const p_currents)
{
#if DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
    const uint8_t buf[3] = { p_currents->current_red, p_currents->current_green, p_currents->current_blue };
    if (!lp5810_auto_animation_enable(dev_lp5810, &buf[0], sizeof(buf)))
    {
        LOG_ERR("LP5810: Failed to set AUTO DC");
        return false;
    }

    const lp5810_auto_animation_cfg_t anim_cfg = {
        .auto_pause    = 0,
        .auto_playback = 0x0F,
        .AEU1_PWM      = { 0, 255, 0, 0, 0 },
        .AEU1_T12      = 0x44,
        .AEU1_T34      = 0x00,
        .AEU1_playback = 0x03,
        .AEU2_PWM      = { 0, 0, 0, 0, 0 },
        .AEU2_T12      = 0,
        .AEU2_T34      = 0,
        .AEU2_playback = 0,
        .AEU3_PWM      = { 0, 0, 0, 0, 0 },
        .AEU3_T12      = 0,
        .AEU3_T34      = 0,
        .AEU3_playback = 0,
    };
    for (int i = 0; i < 3; i++)
    {
        if (!lp5810_auto_animation_configure(dev_lp5810, i, &anim_cfg))
        {
            LOG_ERR("LP5810: Failed to configure AUTO ANIMATION");
            return false;
        }
    }
    if (!lp5810_auto_animation_start(dev_lp5810))
    {
        LOG_ERR("LP5810: Failed to start AUTO ANIMATION");
        return false;
    }
#endif // DT_HAS_COMPAT_STATUS_OKAY(ti_lp5810)
    return true;
}
