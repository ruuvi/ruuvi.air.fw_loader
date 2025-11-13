/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#ifndef FWLOADER_WATCHDOG_H
#define FWLOADER_WATCHDOG_H

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

bool
fwloader_watchdog_start(void);

void
fwloader_watchdog_feed(void);

__NO_RETURN void
fwloader_watchdog_force_trigger(void);

#ifdef __cplusplus
}
#endif

#endif // FWLOADER_WATCHDOG_H
