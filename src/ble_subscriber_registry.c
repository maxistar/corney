#include <errno.h>
#include <limits.h>
#include <string.h>

#include <corney/ble_subscriber_registry.h>

void corney_ble_subscriber_registry_init(
    struct corney_ble_subscriber_registry *registry,
    struct corney_ble_subscriber *slots, size_t capacity) {
  if (registry == NULL) {
    return;
  }
  registry->slots = slots;
  registry->capacity = capacity;
  registry->next_scan = 0U;
  if (slots != NULL && capacity > 0U) {
    memset(slots, 0, sizeof(*slots) * capacity);
  }
}

struct corney_ble_subscriber *
corney_ble_subscriber_find(struct corney_ble_subscriber_registry *registry,
                           const void *connection) {
  size_t index;
  if (registry == NULL || registry->slots == NULL || connection == NULL) {
    return NULL;
  }
  for (index = 0U; index < registry->capacity; index++) {
    if (registry->slots[index].connection == connection) {
      return &registry->slots[index];
    }
  }
  return NULL;
}

const struct corney_ble_subscriber *corney_ble_subscriber_find_const(
    const struct corney_ble_subscriber_registry *registry,
    const void *connection) {
  size_t index;
  if (registry == NULL || registry->slots == NULL || connection == NULL) {
    return NULL;
  }
  for (index = 0U; index < registry->capacity; index++) {
    if (registry->slots[index].connection == connection) {
      return &registry->slots[index];
    }
  }
  return NULL;
}

int corney_ble_subscriber_add(struct corney_ble_subscriber_registry *registry,
                              const void *connection, size_t *slot_index,
                              bool *added) {
  size_t index;
  if (registry == NULL || registry->slots == NULL || registry->capacity == 0U ||
      connection == NULL) {
    return -EINVAL;
  }

  for (index = 0U; index < registry->capacity; index++) {
    if (registry->slots[index].connection == connection) {
      if (slot_index != NULL) {
        *slot_index = index;
      }
      if (added != NULL) {
        *added = false;
      }
      return 0;
    }
  }

  for (index = 0U; index < registry->capacity; index++) {
    if (registry->slots[index].connection == NULL) {
      memset(&registry->slots[index], 0, sizeof(registry->slots[index]));
      registry->slots[index].connection = connection;
      if (slot_index != NULL) {
        *slot_index = index;
      }
      if (added != NULL) {
        *added = true;
      }
      return 0;
    }
  }
  return -ENOSPC;
}

bool corney_ble_subscriber_remove(
    struct corney_ble_subscriber_registry *registry, const void *connection) {
  struct corney_ble_subscriber *subscriber =
      corney_ble_subscriber_find(registry, connection);
  if (subscriber == NULL) {
    return false;
  }
  memset(subscriber, 0, sizeof(*subscriber));
  return true;
}

size_t corney_ble_subscriber_count(
    const struct corney_ble_subscriber_registry *registry) {
  size_t index;
  size_t count = 0U;
  if (registry == NULL || registry->slots == NULL) {
    return 0U;
  }
  for (index = 0U; index < registry->capacity; index++) {
    if (registry->slots[index].connection != NULL) {
      count++;
    }
  }
  return count;
}

size_t corney_ble_subscriber_next_occupied(
    struct corney_ble_subscriber_registry *registry) {
  size_t offset;
  if (registry == NULL || registry->slots == NULL || registry->capacity == 0U) {
    return SIZE_MAX;
  }
  for (offset = 0U; offset < registry->capacity; offset++) {
    size_t index = (registry->next_scan + offset) % registry->capacity;
    if (registry->slots[index].connection != NULL) {
      registry->next_scan = (index + 1U) % registry->capacity;
      return index;
    }
  }
  return SIZE_MAX;
}

static bool subscriber_has_work(struct corney_ble_subscriber *subscriber,
                                uint64_t oldest_entry_id,
                                uint64_t next_entry_id) {
  if (subscriber->state == CORNEY_BLE_SUBSCRIBER_ACTIVE &&
      subscriber->next_entry_id < oldest_entry_id) {
    subscriber->next_entry_id = oldest_entry_id;
    subscriber->retry_count = 0U;
    subscriber->retry_at_ms = 0;
  }
  return subscriber->state == CORNEY_BLE_SUBSCRIBER_INITIALIZING ||
         (subscriber->state == CORNEY_BLE_SUBSCRIBER_ACTIVE &&
          subscriber->next_entry_id < next_entry_id);
}

size_t corney_ble_subscriber_next_ready(
    struct corney_ble_subscriber_registry *registry, uint64_t oldest_entry_id,
    uint64_t next_entry_id, int64_t now_ms) {
  size_t checked;
  if (registry == NULL || registry->slots == NULL) {
    return SIZE_MAX;
  }
  for (checked = 0U; checked < registry->capacity; checked++) {
    size_t index = corney_ble_subscriber_next_occupied(registry);
    struct corney_ble_subscriber *subscriber;
    if (index == SIZE_MAX) {
      return SIZE_MAX;
    }
    subscriber = &registry->slots[index];
    if (subscriber_has_work(subscriber, oldest_entry_id, next_entry_id) &&
        subscriber->retry_at_ms <= now_ms) {
      return index;
    }
  }
  return SIZE_MAX;
}

int64_t corney_ble_subscriber_next_delay_ms(
    struct corney_ble_subscriber_registry *registry, uint64_t oldest_entry_id,
    uint64_t next_entry_id, int64_t now_ms) {
  int64_t earliest = INT64_MAX;
  size_t index;
  if (registry == NULL || registry->slots == NULL) {
    return -1;
  }
  for (index = 0U; index < registry->capacity; index++) {
    struct corney_ble_subscriber *subscriber = &registry->slots[index];
    if (subscriber->connection == NULL ||
        !subscriber_has_work(subscriber, oldest_entry_id, next_entry_id)) {
      continue;
    }
    if (subscriber->retry_at_ms <= now_ms) {
      return 0;
    }
    if (subscriber->retry_at_ms < earliest) {
      earliest = subscriber->retry_at_ms;
    }
  }
  return earliest == INT64_MAX ? -1 : earliest - now_ms;
}
