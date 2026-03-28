/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>

#include <stdint.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define ZMK_GATT_LAYER_SERVICE_UUID                                                               \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x715d81e1, 0x377d, 0x4f26, 0xa678, 0xa506675d99ec))

#define ZMK_GATT_LAYER_CHAR_UUID                                                                  \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xe87c518a, 0xf323, 0x4dd7, 0x9dd5, 0x0991add1c01b))

enum {
    LAYER_SVC_ATTR_PRIMARY,
    LAYER_SVC_ATTR_CHAR_DECL,
    LAYER_SVC_ATTR_CHAR_VALUE,
    LAYER_SVC_ATTR_CHAR_CCC,
    LAYER_SVC_ATTR_COUNT,
};

static bool layer_notify_enabled;
static uint8_t last_layer_index = UINT8_MAX;

static uint8_t current_layer_index(void) {
    return (uint8_t)zmk_keymap_highest_layer_active();
}

static ssize_t read_layer(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                          uint16_t len, uint16_t offset) {
    uint8_t layer = current_layer_index();
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &layer, sizeof(layer));
}

static void layer_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value) {
    ARG_UNUSED(attr);
    layer_notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}

BT_GATT_SERVICE_DEFINE(layer_svc, BT_GATT_PRIMARY_SERVICE(ZMK_GATT_LAYER_SERVICE_UUID),
                       BT_GATT_CHARACTERISTIC(ZMK_GATT_LAYER_CHAR_UUID,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                                              BT_GATT_PERM_READ, read_layer, NULL, NULL),
                       BT_GATT_CCC(layer_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE));

static void notify_layer_change(uint8_t layer_index) {
    if (!layer_notify_enabled) {
        return;
    }

    int err = bt_gatt_notify(NULL, &layer_svc.attrs[LAYER_SVC_ATTR_CHAR_VALUE], &layer_index,
                             sizeof(layer_index));
    if (err) {
        LOG_DBG("Layer notify failed (%d)", err);
    }
}

static int layer_state_changed_listener(const zmk_event_t *eh) {
    if (as_zmk_layer_state_changed(eh) == NULL) {
        return -ENOTSUP;
    }

    uint8_t layer_index = current_layer_index();
    if (layer_index == last_layer_index) {
        return 0;
    }

    last_layer_index = layer_index;
    notify_layer_change(layer_index);
    return 0;
}

ZMK_LISTENER(layer_gatt, layer_state_changed_listener);
ZMK_SUBSCRIPTION(layer_gatt, zmk_layer_state_changed);
