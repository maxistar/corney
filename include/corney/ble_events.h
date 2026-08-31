#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void corney_ble_publish_key(bool pressed, uint8_t position, uint8_t layer);
void corney_ble_publish_combo(uint16_t combo_id, bool pressed, uint8_t layer,
                              const uint8_t *positions, size_t position_count);
void corney_ble_publish_layer(uint8_t layer, uint8_t previous_layer,
                              uint8_t cause, uint8_t origin_position,
                              uint8_t flags);
void corney_ble_publish_initial_snapshots(void);
