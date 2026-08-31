#pragma once

#include <stddef.h>

#include <corney/ble_contract.h>

struct corney_ble_frame_queue {
  struct corney_ble_frame *storage;
  size_t capacity;
  size_t head;
  size_t count;
};

void corney_ble_frame_queue_init(struct corney_ble_frame_queue *queue,
                                 struct corney_ble_frame *storage,
                                 size_t capacity);
int corney_ble_frame_queue_push(struct corney_ble_frame_queue *queue,
                                const struct corney_ble_frame *frame);
int corney_ble_frame_queue_peek(const struct corney_ble_frame_queue *queue,
                                struct corney_ble_frame *frame);
int corney_ble_frame_queue_pop(struct corney_ble_frame_queue *queue,
                               struct corney_ble_frame *frame);
void corney_ble_frame_queue_purge(struct corney_ble_frame_queue *queue);
size_t corney_ble_frame_queue_count(const struct corney_ble_frame_queue *queue);
