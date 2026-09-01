#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <corney/ble_contract.h>

struct corney_ble_history_entry {
  uint64_t id;
  struct corney_ble_frame frame;
};

struct corney_ble_frame_history {
  struct corney_ble_history_entry *storage;
  size_t capacity;
  size_t head;
  size_t count;
  uint64_t next_id;
};

void corney_ble_frame_history_init(struct corney_ble_frame_history *history,
                                   struct corney_ble_history_entry *storage,
                                   size_t capacity);
int corney_ble_frame_history_push(struct corney_ble_frame_history *history,
                                  const struct corney_ble_frame *frame,
                                  uint64_t *entry_id, bool *overwrote_oldest);
int corney_ble_frame_history_get(const struct corney_ble_frame_history *history,
                                 uint64_t entry_id,
                                 struct corney_ble_frame *frame);
void corney_ble_frame_history_discard_before(
    struct corney_ble_frame_history *history, uint64_t first_retained_id);
void corney_ble_frame_history_purge(struct corney_ble_frame_history *history);
uint64_t corney_ble_frame_history_oldest_id(
    const struct corney_ble_frame_history *history);
uint64_t corney_ble_frame_history_next_id(
    const struct corney_ble_frame_history *history);
size_t
corney_ble_frame_history_count(const struct corney_ble_frame_history *history);
