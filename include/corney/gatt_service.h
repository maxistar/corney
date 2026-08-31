#pragma once

#include <stdint.h>

#include <zephyr/bluetooth/conn.h>

#include <corney/ble_contract.h>

int corney_gatt_notify_legacy_layer(int32_t layer);
int corney_gatt_notify_event(struct bt_conn *conn,
                             const struct corney_ble_frame *frame);
const struct bt_gatt_attr *corney_gatt_event_attr(void);
