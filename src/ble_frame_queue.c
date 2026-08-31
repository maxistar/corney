#include <errno.h>

#include <corney/ble_frame_queue.h>

void corney_ble_frame_queue_init(struct corney_ble_frame_queue *queue,
                                 struct corney_ble_frame *storage,
                                 size_t capacity) {
  queue->storage = storage;
  queue->capacity = capacity;
  queue->head = 0;
  queue->count = 0;
}

int corney_ble_frame_queue_push(struct corney_ble_frame_queue *queue,
                                const struct corney_ble_frame *frame) {
  size_t tail;
  if (queue == NULL || frame == NULL || queue->storage == NULL ||
      queue->capacity == 0U) {
    return -EINVAL;
  }
  if (queue->count == queue->capacity) {
    return -ENOSPC;
  }
  tail = (queue->head + queue->count) % queue->capacity;
  queue->storage[tail] = *frame;
  queue->count++;
  return 0;
}

int corney_ble_frame_queue_peek(const struct corney_ble_frame_queue *queue,
                                struct corney_ble_frame *frame) {
  if (queue == NULL || frame == NULL || queue->count == 0U) {
    return -ENOENT;
  }
  *frame = queue->storage[queue->head];
  return 0;
}

int corney_ble_frame_queue_pop(struct corney_ble_frame_queue *queue,
                               struct corney_ble_frame *frame) {
  int err = corney_ble_frame_queue_peek(queue, frame);
  if (err != 0) {
    return err;
  }
  queue->head = (queue->head + 1U) % queue->capacity;
  queue->count--;
  return 0;
}

void corney_ble_frame_queue_purge(struct corney_ble_frame_queue *queue) {
  if (queue != NULL) {
    queue->head = 0;
    queue->count = 0;
  }
}

size_t
corney_ble_frame_queue_count(const struct corney_ble_frame_queue *queue) {
  return queue == NULL ? 0U : queue->count;
}
