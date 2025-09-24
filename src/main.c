/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2020 Prevas A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/stats/stats.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_MCUMGR_GRP_FS
#include <zephyr/device.h>
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
#include "app_version.h"
#include "ncs_version.h"
#include "version.h"
#if APP_VERSION_NUMBER != 0
#include "app_commit.h"
#endif
#include "ncs_commit.h"
#include "zephyr_commit.h"

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
#if defined(CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION)
    LOG_INF(
        "### Ruuvi FwLoader: Image version: %s (FwInfoCnt: %u)",
        CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION,
        CONFIG_FW_INFO_FIRMWARE_VERSION);
#endif
#if APP_VERSION_NUMBER != 0
    LOG_INF(
        "### Ruuvi FwLoader: Version: %s, build: %s, commit: %s",
        APP_VERSION_EXTENDED_STRING,
        STRINGIFY(APP_BUILD_VERSION),
        APP_COMMIT_STRING);
#else
    LOG_INF("### Ruuvi FwLoader: Version: %s, build: %s", APP_VERSION_EXTENDED_STRING, STRINGIFY(APP_BUILD_VERSION));
#endif
    LOG_INF(
        "### Ruuvi FwLoader: NCS version: %s, build: %s, commit: %s",
        NCS_VERSION_STRING,
        STRINGIFY(NCS_BUILD_VERSION),
        NCS_COMMIT_STRING);
    LOG_INF(
        "### Ruuvi FwLoader: Kernel version: %s, build: %s, commit: %s",
        KERNEL_VERSION_EXTENDED_STRING,
        STRINGIFY(BUILD_VERSION),
        ZEPHYR_COMMIT_STRING);

    fwloader_segger_rtt_check_data_location_and_size();

#ifdef CONFIG_MCUMGR_GRP_FS
    btldr_fs_mount();
#endif

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

    /* The system work queue handles all incoming mcumgr requests.  Let the
     * main thread idle while the mcumgr server runs.
     */
    while (1)
    {
        fwloader_led_lock();
        if (!fwloader_button_is_pressed())
        {
            fwloader_led_green_on();
            fwloader_led_red_on();
        }
        fwloader_led_unlock();

        k_sleep(K_MSEC(500));

        fwloader_led_lock();
        if (!fwloader_button_is_pressed())
        {
            fwloader_led_green_off();
            fwloader_led_red_off();
        }
        fwloader_led_unlock();

        k_sleep(K_MSEC(500));
        STATS_INC(smp_svr_stats, ticks);
    }
    return 0;
}
