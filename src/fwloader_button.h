/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#ifndef FWLOADER_BUTTON_H
#define FWLOADER_BUTTON_H

#include <stdbool.h>
#include <zephyr/drivers/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

void
fwloader_button_init(
    struct gpio_callback* const   p_gpio_callback,
    const gpio_callback_handler_t cb_handler,
    const gpio_flags_t            int_flags);

void
fwloader_button_deinit(struct gpio_callback* const p_gpio_callback);

void
fwloader_button_int_disable(void);

void
fwloader_button_remove_cb(struct gpio_callback* const p_gpio_callback);

bool
fwloader_button_get(void);

void
fwloader_button_set_pressed(void);

bool
fwloader_button_is_pressed(void);

#ifdef __cplusplus
}
#endif

#endif // FWLOADER_BUTTON_H
