#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/bluetooth/conn.h>

#include <corney/ble_contract.h>

int corney_ble_transport_claim(struct bt_conn *conn);
void corney_ble_transport_release(struct bt_conn *conn);
bool corney_ble_transport_is_owner(struct bt_conn *conn);
struct bt_conn *corney_ble_transport_owner(void);
void corney_ble_transport_start_epoch(void);
void corney_ble_transport_finish_epoch(void);
int corney_ble_transport_publish(uint8_t type, uint8_t flags,
                                 const uint8_t *payload, size_t payload_len);
int corney_ble_transport_publish_frame(struct corney_ble_frame *frame);
uint32_t corney_ble_transport_drop_count(void);
