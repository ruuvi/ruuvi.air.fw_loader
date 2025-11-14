/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2020 Prevas A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/stats/stats.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/sys/__assert.h>

#ifdef CONFIG_MCUMGR_GRP_FS
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#endif
#ifdef CONFIG_MCUMGR_GRP_STAT
#include <zephyr/mgmt/mcumgr/grp/stat_mgmt/stat_mgmt.h>
#endif
#include "fwloader_segger_rtt.h"
#include "fwloader_button_cb.h"
#include "fwloader_button.h"
#include "fwloader_led.h"
#include "fwloader_bluetooth.h"
#include "fwloader_watchdog.h"
#include "fwloader_mcumgr_mgmt_callbacks.h"
#include "fwloader_settings.h"
#include "fwloader_fw_ver.h"

LOG_MODULE_REGISTER(fw_loader, LOG_LEVEL_INF);

#define STORAGE_PARTITION_LABEL storage_partition
#define STORAGE_PARTITION_ID    FIXED_PARTITION_ID(STORAGE_PARTITION_LABEL)

#define HIST_LOG_PARTITION_LABEL hist_storage
#define HIST_LOG_PARTITION_SIZE  PM_HIST_STORAGE_SIZE

#define HIST_LOG_FLASH_AREA_ID FLASH_AREA_ID(HIST_LOG_PARTITION_LABEL)

/* Define an example stats group; approximates seconds since boot. */
STATS_SECT_START(smp_svr_stats)
STATS_SECT_ENTRY(ticks)
STATS_SECT_END;

/* Assign a name to the `ticks` stat. */
STATS_NAME_START(smp_svr_stats)
STATS_NAME(smp_svr_stats, ticks)
STATS_NAME_END(smp_svr_stats);

/* Define an instance of the stats group. */
STATS_SECT_DECL(smp_svr_stats) smp_svr_stats;

#ifdef CONFIG_MCUMGR_GRP_FS
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t btldr_fs_storage_mnt = {
    .type        = FS_LITTLEFS,
    .fs_data     = &storage,
    .storage_dev = (void*)FIXED_PARTITION_ID(littlefs_storage1),
    .mnt_point   = "/lfs1",
};

static struct fs_mount_t* const g_mountpoint = &btldr_fs_storage_mnt;
#endif

#ifdef CONFIG_MCUMGR_GRP_FS
static bool
btldr_fs_mount(void)
{
    int rc = fs_mkfs(FS_LITTLEFS, FIXED_PARTITION_ID(littlefs_storage1), NULL, 0);
    if (0 != rc)
    {
        LOG_ERR("FAIL: mkfs fa_id %d: res=%d", FIXED_PARTITION_ID(littlefs_storage1), rc);
        return false;
    }

    rc = fs_mount(g_mountpoint);
    if (0 != rc)
    {
        LOG_ERR(
            "FAIL: mount id %" PRIuPTR " at %s: %d",
            (uintptr_t)g_mountpoint->storage_dev,
            g_mountpoint->mnt_point,
            rc);
        return false;
    }
    LOG_INF("%s mounted successfully", g_mountpoint->mnt_point);

    struct fs_statvfs sbuf = { 0 };
    rc                     = fs_statvfs(g_mountpoint->mnt_point, &sbuf);
    if (rc < 0)
    {
        LOG_ERR("FAIL: statvfs: %d", rc);
        return false;
    }
    LOG_INF(
        "%s: bsize = %lu ; frsize = %lu ; blocks = %lu ; bfree = %lu",
        g_mountpoint->mnt_point,
        sbuf.f_bsize,
        sbuf.f_frsize,
        sbuf.f_blocks,
        sbuf.f_bfree);
    return true;
}
#endif

int
main(void)
{
    fwloader_fw_ver_init();

    fwloader_segger_rtt_check_data_location_and_size();

    if (!fwloader_settings_init())
    {
        LOG_ERR("fwloader_settings_init failed");
    }

#ifdef CONFIG_MCUMGR_GRP_FS
    btldr_fs_mount();
#endif

    if (!fwloader_watchdog_start())
    {
        LOG_ERR("Failed to initialize watchdog");
    }

    int rc = STATS_INIT_AND_REG(smp_svr_stats, STATS_SIZE_32, "smp_svr_stats");
    if (rc < 0)
    {
        LOG_ERR("Error initializing stats system [%d]", rc);
    }

    /* Register the built-in mcumgr command handlers. */
#ifdef CONFIG_MCUMGR_TRANSPORT_BT
    LOG_INF("fw_loader: start_smp_bluetooth_adverts");
    start_smp_bluetooth_adverts();
#endif
    fwloader_mcumgr_mgmt_callbacks_init(btldr_fs_storage_mnt.mnt_point);

    fwloader_led_lock();
    if (!fwloader_button_is_pressed())
    {
        fwloader_led_red_on();
    }
    fwloader_led_unlock();

    /* The system work queue handles all incoming mcumgr requests.  Let the
     * main thread idle while the mcumgr server runs.
     */
    uint32_t uploading_timeout_cnt = 0;
    while (1)
    {
        if (!fwloader_button_is_pressed())
        {
            fwloader_watchdog_feed();
        }

        if (fwloader_bt_is_connected())
        {
            if (fwloader_mcumgr_mgmt_callbacks_is_uploading_in_progress())
            {
                uploading_timeout_cnt = 0;
            }
            else
            {
                uploading_timeout_cnt++;
            }
            LOG_INF("Uploading is in progress, timeout cnt: %d", uploading_timeout_cnt);
        }
        else
        {
            uploading_timeout_cnt++;
            LOG_INF("Uploading is not in progress, timeout cnt: %d", uploading_timeout_cnt);
        }
        if (uploading_timeout_cnt >= CONFIG_RUUVI_AIR_FW_LOADER_UPLOADING_TIMEOUT_SEC)
        {
            LOG_WRN("No upload activity for %d seconds, rebooting", CONFIG_RUUVI_AIR_FW_LOADER_UPLOADING_TIMEOUT_SEC);
            fwloader_watchdog_force_trigger();
        }

        k_sleep(K_MSEC(1000));
        STATS_INC(smp_svr_stats, ticks);
    }
    return 0;
}

/**
 * Declare the symbol pointing to the former implementation of sys_reboot function
 */
extern void
__real_sys_reboot(int type);

/**
 * Redefine sys_reboot function to print a message before actually restarting
 */
void
__wrap_sys_reboot(int type) // NOSONAR
{
    LOG_WRN("Rebooting...");
    k_msleep(25); // Give some time to print log message

    /* Call the former implementation to actually restart the board */
#if CONFIG_DEBUG
    __real_sys_reboot(type);
#else
    fwloader_watchdog_force_trigger();
#endif
}
