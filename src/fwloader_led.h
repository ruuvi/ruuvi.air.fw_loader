/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#if !defined(FWLOADER_LED_H)
#define FWLOADER_LED_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void
fwloader_led_init(void);

void
fwloader_led_deinit(void);

void
fwloader_led_red_set(const bool is_on);

static inline void
fwloader_led_red_on(void)
{
    fwloader_led_red_set(true);
}

static inline void
fwloader_led_red_off(void)
{
    fwloader_led_red_set(false);
}

void
fwloader_led_green_set(const bool is_on);

static inline void
fwloader_led_green_on(void)
{
    fwloader_led_green_set(true);
}

static inline void
fwloader_led_green_off(void)
{
    fwloader_led_green_set(false);
}

void
fwloader_led_lock(void);

void
fwloader_led_unlock(void);

#ifdef __cplusplus
}
#endif

#endif // FWLOADER_LED_H
