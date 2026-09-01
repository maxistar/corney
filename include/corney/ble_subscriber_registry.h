#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum corney_ble_subscriber_state {
  CORNEY_BLE_SUBSCRIBER_EMPTY = 0,
  CORNEY_BLE_SUBSCRIBER_INITIALIZING,
  CORNEY_BLE_SUBSCRIBER_ACTIVE,
};

struct corney_ble_subscriber {
  const void *connection;
  uint64_t next_entry_id;
  uint32_t snapshot_sequence;
  int64_t retry_at_ms;
  uint8_t retry_count;
  enum corney_ble_subscriber_state state;
  bool stream_start_pending;
};

struct corney_ble_subscriber_registry {
  struct corney_ble_subscriber *slots;
  size_t capacity;
  size_t next_scan;
};

void corney_ble_subscriber_registry_init(
    struct corney_ble_subscriber_registry *registry,
    struct corney_ble_subscriber *slots, size_t capacity);
int corney_ble_subscriber_add(struct corney_ble_subscriber_registry *registry,
                              const void *connection, size_t *slot_index,
                              bool *added);
bool corney_ble_subscriber_remove(
    struct corney_ble_subscriber_registry *registry, const void *connection);
struct corney_ble_subscriber *
corney_ble_subscriber_find(struct corney_ble_subscriber_registry *registry,
                           const void *connection);
const struct corney_ble_subscriber *corney_ble_subscriber_find_const(
    const struct corney_ble_subscriber_registry *registry,
    const void *connection);
size_t corney_ble_subscriber_count(
    const struct corney_ble_subscriber_registry *registry);
size_t corney_ble_subscriber_next_occupied(
    struct corney_ble_subscriber_registry *registry);
size_t corney_ble_subscriber_next_ready(
    struct corney_ble_subscriber_registry *registry, uint64_t oldest_entry_id,
    uint64_t next_entry_id, int64_t now_ms);
int64_t corney_ble_subscriber_next_delay_ms(
    struct corney_ble_subscriber_registry *registry, uint64_t oldest_entry_id,
    uint64_t next_entry_id, int64_t now_ms);
