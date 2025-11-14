/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#ifndef FWLOADER_FW_VER_H
#define FWLOADER_FW_VER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void
fwloader_fw_ver_init(void);

const char*
fwloader_fw_ver_get(void);

const char*
fwloader_hw_rev_get(void);

#ifdef __cplusplus
}
#endif

#endif // FWLOADER_FW_VER_H
