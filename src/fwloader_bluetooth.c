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
#include "sys_utils.h"
#include "fwloader_led.h"
#include "fwloader_button.h"
#include "zephyr_api.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smp_bt_sample);

static struct k_work advertise_work;

static char     g_bt_name[sizeof(CONFIG_BT_DEVICE_NAME) + 5];
static uint64_t g_ble_mac;
static bool     g_bt_is_connected;

#define NUM_RECORDS_IN_ADVS_PACKET     2
#define NUM_RECORDS_IN_SCAN_RSP_PACKET 1

static const struct bt_data ad[NUM_RECORDS_IN_ADVS_PACKET] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, SMP_BT_SVC_UUID_VAL),
};

static const struct bt_data sd[NUM_RECORDS_IN_SCAN_RSP_PACKET] = {
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
        for (uint32_t i = 0; i < BT_ADDR_SIZE; ++i)
        {
            mac |= (uint64_t)addr[0].a.val[i] << (i * BITS_PER_BYTE);
        }
    }
    (void)snprintf(
        g_bt_name,
        sizeof(g_bt_name),
        "%s %02X%02X",
        CONFIG_BT_DEVICE_NAME,
        addr[0].a.val[BYTE_IDX_1],
        addr[0].a.val[BYTE_IDX_0]);
    LOG_INF(
        "BLE MAC: %02x:%02x:%02x:%02x:%02x:%02x",
        (uint8_t)mac & BYTE_MASK,
        (uint8_t)((mac >> BYTE_SHIFT_1) & BYTE_MASK),
        (uint8_t)((mac >> BYTE_SHIFT_2) & BYTE_MASK),
        (uint8_t)((mac >> BYTE_SHIFT_3) & BYTE_MASK),
        (uint8_t)((mac >> BYTE_SHIFT_4) & BYTE_MASK),
        (uint8_t)((mac >> BYTE_SHIFT_5) & BYTE_MASK));
    return mac;
}

static void
advertise(__unused struct k_work* work)
{
    bt_le_adv_stop();

    const struct bt_le_adv_param adv_param = BT_LE_ADV_PARAM_INIT(
        BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL);

    const zephyr_api_ret_t rc = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (0 != rc)
    {
        LOG_ERR("Advertising failed to start (rc %d)", rc);
        return;
    }

    LOG_INF("Advertising successfully started");
}

static void
connected(__unused struct bt_conn* conn, uint8_t err)
{
    if (0 != err)
    {
        LOG_ERR("Connection failed (err 0x%02x)", err);
    }
    else
    {
        LOG_INF("Connected");
        g_bt_is_connected = true;
        fwloader_led_lock();
        if (!fwloader_button_is_pressed())
        {
            fwloader_led_red_off();
            fwloader_led_green_on();
        }
        fwloader_led_unlock();
    }
}

static void
disconnected(__unused struct bt_conn* conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason 0x%02x)", reason);
    g_bt_is_connected = false;
    fwloader_led_lock();
    if (!fwloader_button_is_pressed())
    {
        fwloader_led_green_off();
        fwloader_led_red_on();
    }
    fwloader_led_unlock();
    k_work_submit(&advertise_work);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected    = connected,
    .disconnected = disconnected,
};

static void
bt_ready(int err /* NOSONAR: Zephyr API */)
{
    LOG_INF("bt_ready");
    if (0 != err)
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
    LOG_INF("start_smp_bluetooth_adverts");

    k_work_init(&advertise_work, advertise);
    zephyr_api_ret_t rc = bt_enable(bt_ready);

    if (rc != 0)
    {
        LOG_ERR("Bluetooth enable failed: %d", rc);
    }
}

bool
fwloader_bt_is_connected(void)
{
    return g_bt_is_connected;
}
