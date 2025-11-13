/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#ifndef FWLOADER_BLUETOOTH_H
#define FWLOADER_BLUETOOTH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void
start_smp_bluetooth_adverts(void);

bool
fwloader_bt_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif // FWLOADER_BLUETOOTH_H
