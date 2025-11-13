/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#ifndef FWLOADER_MGMT_CALLBACKS_H
#define FWLOADER_MGMT_CALLBACKS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void
fwloader_mcumgr_mgmt_callbacks_init(const char* const p_mnt_point);

bool
fwloader_mcumgr_mgmt_callbacks_is_uploading_in_progress(void);

#ifdef __cplusplus
}
#endif

#endif // FWLOADER_MGMT_CALLBACKS_H
