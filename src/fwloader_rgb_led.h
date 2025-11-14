/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#ifndef RUUVI_FWLLOADER_RGB_LED_H
#define RUUVI_FWLLOADER_RGB_LED_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t fwloader_rgb_led_current_t; //<! Current value 0..255
typedef uint8_t fwloader_rgb_led_pwm_t;     //<! PWM value 0..255

typedef struct fwloader_rgb_led_pwms_t
{
    fwloader_rgb_led_pwm_t pwm_red;   //<! PWM value for red LED 0..255
    fwloader_rgb_led_pwm_t pwm_green; //<! PWM value for green LED 0..255
    fwloader_rgb_led_pwm_t pwm_blue;  //<! PWM value for blue LED 0..255
} fwloader_rgb_led_pwms_t;

typedef struct fwloader_rgb_led_currents_t
{
    fwloader_rgb_led_current_t current_red;   //<! Current value for red LED 0..255
    fwloader_rgb_led_current_t current_green; //<! Current value for green LED 0..255
    fwloader_rgb_led_current_t current_blue;  //<! Current value for blue LED 0..255
} fwloader_rgb_led_currents_t;

bool
fwloader_rgb_led_is_lp5810_ready(void);

bool
fwloader_rgb_led_set_raw_currents_and_pwms(
    const fwloader_rgb_led_currents_t* const p_rgb_led_currents,
    const fwloader_rgb_led_pwms_t* const     p_rgb_led_pwms);

bool
fwloader_rgb_led_update_pwms(const fwloader_rgb_led_pwms_t* const p_rgb_led_pwms);

bool
fwloader_rgb_led_check_and_reinit_if_needed(void);

bool
fwloader_rgb_led_turn_on_animation_blinking_white(const fwloader_rgb_led_currents_t* const p_currents);

#ifdef __cplusplus
}
#endif

#endif // RUUVI_FWLLOADER_RGB_LED_H
