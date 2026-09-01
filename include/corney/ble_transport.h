#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/bluetooth/conn.h>

#include <corney/ble_contract.h>

int corney_ble_transport_subscribe(struct bt_conn *conn);
void corney_ble_transport_unsubscribe(struct bt_conn *conn);
bool corney_ble_transport_is_subscriber(struct bt_conn *conn);
int corney_ble_transport_publish(uint8_t type, uint8_t flags,
                                 const uint8_t *payload, size_t payload_len);
uint32_t corney_ble_transport_drop_count(void);
