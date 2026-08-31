#pragma once

#include <stdint.h>

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

ssize_t corney_layer_read(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset);
ssize_t corney_layer_write(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr, const void *buf,
                           uint16_t len, uint16_t offset, uint8_t flags);
void corney_layer_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value);
uint8_t corney_layer_current(void);
