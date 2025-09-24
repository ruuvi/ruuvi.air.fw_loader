/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2020 Prevas A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fwloader_bluetooth.h"
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/mgmt/mcumgr/transport/smp_bt.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smp_bt_sample);

static struct k_work advertise_work;

static char     g_bt_name[sizeof(CONFIG_BT_DEVICE_NAME) + 5];
static uint64_t g_ble_mac;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, SMP_BT_SVC_UUID_VAL),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, g_bt_name, sizeof(g_bt_name)),
};

uint64_t
radio_address_get(void)
{
    uint64_t mac = 0;

    bt_addr_le_t addr[CONFIG_BT_ID_MAX];
    size_t       count = CONFIG_BT_ID_MAX;
    bt_id_get(addr, &count);

    if (count > 0)
    {
        for (int i = 0; i < 6; i++)
        {
            mac |= (uint64_t)addr[0].a.val[i] << (i * 8);
        }
    }
    (void)snprintf(
        g_bt_name,
        sizeof(g_bt_name),
        "%s %02X%02X",
        CONFIG_BT_DEVICE_NAME,
        addr[0].a.val[1],
        addr[0].a.val[0]);
    LOG_INF(
        "BLE MAC: %02x:%02x:%02x:%02x:%02x:%02x",
        (uint8_t)mac & 0xFF,
        (uint8_t)((mac >> 8) & 0xFF),
        (uint8_t)((mac >> 16) & 0xFF),
        (uint8_t)((mac >> 24) & 0xFF),
        (uint8_t)((mac >> 32) & 0xFF),
        (uint8_t)((mac >> 40) & 0xFF));
    return mac;
}

static void
advertise(struct k_work* work)
{
    bt_le_adv_stop();

    const struct bt_le_adv_param adv_param = BT_LE_ADV_PARAM_INIT(
        BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL);

    const int rc = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (rc)
    {
        LOG_ERR("Advertising failed to start (rc %d)", rc);
        return;
    }

    LOG_INF("Advertising successfully started");
}

static void
connected(struct bt_conn* conn, uint8_t err)
{
    if (err)
    {
        LOG_ERR("Connection failed (err 0x%02x)", err);
    }
    else
    {
        LOG_INF("Connected");
    }
}

static void
disconnected(struct bt_conn* conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason 0x%02x)", reason);
    k_work_submit(&advertise_work);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected    = connected,
    .disconnected = disconnected,
};

static void
bt_ready(int err)
{
    LOG_INF("bt_ready");
    if (err != 0)
    {
        LOG_ERR("Bluetooth failed to initialise: %d", err);
    }
    else
    {
        g_ble_mac = radio_address_get();
        k_work_submit(&advertise_work);
    }
}

void
start_smp_bluetooth_adverts(void)
{
    int rc;

    LOG_INF("start_smp_bluetooth_adverts");

    k_work_init(&advertise_work, advertise);
    rc = bt_enable(bt_ready);

    if (rc != 0)
    {
        LOG_ERR("Bluetooth enable failed: %d", rc);
    }
}
