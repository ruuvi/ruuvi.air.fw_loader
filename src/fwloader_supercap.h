/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#if !defined(FWLOADER_SUPERCAP_H)
#define FWLOADER_SUPERCAP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_BOARD_RUUVI_RUUVIAIR_REV_1)

void
fwloader_supercap_init(void);

#endif // CONFIG_BOARD_RUUVI_RUUVIAIR_REV_1

#ifdef __cplusplus
}
#endif

#endif // FWLOADER_SUPERCAP_H
