#include <errno.h>

#include <corney/ble_frame_history.h>

void corney_ble_frame_history_init(struct corney_ble_frame_history *history,
                                   struct corney_ble_history_entry *storage,
                                   size_t capacity) {
  if (history == NULL) {
    return;
  }
  history->storage = storage;
  history->capacity = capacity;
  history->head = 0U;
  history->count = 0U;
  history->next_id = 0U;
}

int corney_ble_frame_history_push(struct corney_ble_frame_history *history,
                                  const struct corney_ble_frame *frame,
                                  uint64_t *entry_id, bool *overwrote_oldest) {
  size_t tail;
  bool overwrote = false;
  if (history == NULL || frame == NULL || history->storage == NULL ||
      history->capacity == 0U) {
    return -EINVAL;
  }

  if (history->count == history->capacity) {
    history->head = (history->head + 1U) % history->capacity;
    history->count--;
    overwrote = true;
  }
  tail = (history->head + history->count) % history->capacity;
  history->storage[tail].id = history->next_id;
  history->storage[tail].frame = *frame;
  if (entry_id != NULL) {
    *entry_id = history->next_id;
  }
  history->next_id++;
  history->count++;
  if (overwrote_oldest != NULL) {
    *overwrote_oldest = overwrote;
  }
  return 0;
}

int corney_ble_frame_history_get(const struct corney_ble_frame_history *history,
                                 uint64_t entry_id,
                                 struct corney_ble_frame *frame) {
  uint64_t oldest;
  uint64_t offset;
  size_t index;
  if (history == NULL || frame == NULL || history->storage == NULL) {
    return -EINVAL;
  }
  if (history->count == 0U || entry_id >= history->next_id) {
    return -ENOENT;
  }
  oldest = history->storage[history->head].id;
  if (entry_id < oldest) {
    return -ERANGE;
  }
  offset = entry_id - oldest;
  if (offset >= history->count) {
    return -ENOENT;
  }
  index = (history->head + (size_t)offset) % history->capacity;
  if (history->storage[index].id != entry_id) {
    return -ENOENT;
  }
  *frame = history->storage[index].frame;
  return 0;
}

void corney_ble_frame_history_discard_before(
    struct corney_ble_frame_history *history, uint64_t first_retained_id) {
  if (history == NULL || history->storage == NULL) {
    return;
  }
  while (history->count > 0U &&
         history->storage[history->head].id < first_retained_id) {
    history->head = (history->head + 1U) % history->capacity;
    history->count--;
  }
}

void corney_ble_frame_history_purge(struct corney_ble_frame_history *history) {
  if (history != NULL) {
    history->head = 0U;
    history->count = 0U;
  }
}

uint64_t corney_ble_frame_history_oldest_id(
    const struct corney_ble_frame_history *history) {
  if (history == NULL || history->storage == NULL || history->count == 0U) {
    return history == NULL ? 0U : history->next_id;
  }
  return history->storage[history->head].id;
}

uint64_t corney_ble_frame_history_next_id(
    const struct corney_ble_frame_history *history) {
  return history == NULL ? 0U : history->next_id;
}

size_t
corney_ble_frame_history_count(const struct corney_ble_frame_history *history) {
  return history == NULL ? 0U : history->count;
}
