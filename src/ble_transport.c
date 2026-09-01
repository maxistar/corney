#include <errno.h>
#include <limits.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>

#include <corney/ble_contract.h>
#include <corney/ble_events.h>
#include <corney/ble_frame_history.h>
#include <corney/ble_subscriber_registry.h>
#include <corney/ble_transport.h>
#include <corney/ble_transport_policy.h>
#include <corney/gatt_service.h>

LOG_MODULE_REGISTER(corney_ble_transport, LOG_LEVEL_INF);

BUILD_ASSERT(CONFIG_BT_MAX_CONN >= CONFIG_ZMK_SPLIT_BLE_CENTRAL_PERIPHERALS + 2,
             "Keyboard Helper requires two host connections in addition to "
             "split peripherals");

static struct corney_ble_history_entry
    history_storage[CONFIG_ZMK_KEYBOARD_HELPER_EVENT_QUEUE_SIZE];
static struct corney_ble_frame_history event_history = {
    .storage = history_storage,
    .capacity = CONFIG_ZMK_KEYBOARD_HELPER_EVENT_QUEUE_SIZE,
};
static struct corney_ble_subscriber subscriber_slots[CONFIG_BT_MAX_CONN];
static struct corney_ble_subscriber_registry subscribers = {
    .slots = subscriber_slots,
    .capacity = CONFIG_BT_MAX_CONN,
};
static struct k_spinlock transport_lock;
static atomic_t next_sequence = ATOMIC_INIT(CORNEY_BLE_STREAM_INITIAL_SEQUENCE);
static atomic_t queue_drop_count = ATOMIC_INIT(0);
static atomic_t transport_drop_count = ATOMIC_INIT(0);
static atomic_t last_dropped_sequence = ATOMIC_INIT(0);
static atomic_t overflow_report_pending = ATOMIC_INIT(0);

struct transport_candidate {
  struct bt_conn *conn;
  size_t slot_index;
  uint64_t entry_id;
  uint32_t snapshot_sequence;
  bool snapshot;
  bool stream_start_pending;
  struct corney_ble_frame frame;
};

static void transport_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(transport_work, transport_work_handler);

/* Call only while transport_lock is held so subscription baselines and
 * retained-history order share one atomic boundary. */
static uint32_t allocate_sequence_locked(void) {
  return (uint32_t)atomic_inc(&next_sequence);
}

static uint32_t frame_sequence(const struct corney_ble_frame *frame) {
  return (uint32_t)frame->data[4] | ((uint32_t)frame->data[5] << 8) |
         ((uint32_t)frame->data[6] << 16) | ((uint32_t)frame->data[7] << 24);
}

static void record_queue_drop(uint32_t sequence) {
  atomic_inc(&queue_drop_count);
  atomic_set(&last_dropped_sequence, (atomic_val_t)sequence);
  atomic_set(&overflow_report_pending, 1);
}

static uint64_t minimum_subscriber_cursor_locked(void) {
  uint64_t minimum = corney_ble_frame_history_next_id(&event_history);
  size_t index;
  for (index = 0U; index < subscribers.capacity; index++) {
    const struct corney_ble_subscriber *subscriber = &subscribers.slots[index];
    if (subscriber->connection != NULL && subscriber->next_entry_id < minimum) {
      minimum = subscriber->next_entry_id;
    }
  }
  return minimum;
}

static void advance_overwritten_cursors_locked(uint64_t oldest_id) {
  size_t index;
  for (index = 0U; index < subscribers.capacity; index++) {
    struct corney_ble_subscriber *subscriber = &subscribers.slots[index];
    if (subscriber->connection != NULL &&
        subscriber->next_entry_id < oldest_id) {
      subscriber->next_entry_id = oldest_id;
      subscriber->retry_count = 0U;
      subscriber->retry_at_ms = 0;
    }
  }
}

int corney_ble_transport_subscribe(struct bt_conn *conn) {
  struct bt_conn *held;
  struct corney_ble_subscriber *subscriber;
  size_t slot_index;
  bool added = false;
  int err;
  k_spinlock_key_t key;

  if (conn == NULL) {
    return -EINVAL;
  }
  held = bt_conn_ref(conn);
  key = k_spin_lock(&transport_lock);
  err = corney_ble_subscriber_add(&subscribers, conn, &slot_index, &added);
  if (err == 0 && added) {
    subscriber = &subscribers.slots[slot_index];
    subscriber->next_entry_id =
        corney_ble_frame_history_next_id(&event_history);
    subscriber->snapshot_sequence = (uint32_t)atomic_get(&next_sequence) - 1U;
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_LAYER_EVENTS)
    subscriber->state = CORNEY_BLE_SUBSCRIBER_INITIALIZING;
    subscriber->stream_start_pending = false;
#else
    subscriber->state = CORNEY_BLE_SUBSCRIBER_ACTIVE;
    subscriber->stream_start_pending = true;
#endif
  }
  k_spin_unlock(&transport_lock, key);

  if (err != 0 || !added) {
    bt_conn_unref(held);
  } else {
    k_work_reschedule(&transport_work, K_MSEC(1));
  }
  return err;
}

void corney_ble_transport_unsubscribe(struct bt_conn *conn) {
  bool removed;
  bool empty;
  k_spinlock_key_t key;
  if (conn == NULL) {
    return;
  }
  key = k_spin_lock(&transport_lock);
  removed = corney_ble_subscriber_remove(&subscribers, conn);
  empty = corney_ble_subscriber_count(&subscribers) == 0U;
  if (empty) {
    corney_ble_frame_history_purge(&event_history);
  }
  k_spin_unlock(&transport_lock, key);
  if (removed) {
    bt_conn_unref(conn);
  }
  if (empty) {
    k_work_cancel_delayable(&transport_work);
  }
}

bool corney_ble_transport_is_subscriber(struct bt_conn *conn) {
  bool subscribed;
  k_spinlock_key_t key = k_spin_lock(&transport_lock);
  subscribed = corney_ble_subscriber_find_const(&subscribers, conn) != NULL;
  k_spin_unlock(&transport_lock, key);
  return subscribed;
}

uint32_t corney_ble_transport_drop_count(void) {
  return (uint32_t)(atomic_get(&queue_drop_count) +
                    atomic_get(&transport_drop_count));
}

static int publish_frame_locked(const struct corney_ble_frame *frame,
                                bool *overwrote,
                                uint32_t *overwritten_sequence) {
  struct corney_ble_frame overwritten_frame;
  uint64_t oldest_before;
  uint64_t oldest_after;
  bool did_overwrite = false;
  bool had_overwritten_frame = false;
  int err;

  if (frame == NULL || corney_ble_validate_frame(frame) != 0) {
    return -EINVAL;
  }
  if (corney_ble_subscriber_count(&subscribers) == 0U) {
    return -ENOTCONN;
  }

  corney_ble_frame_history_discard_before(&event_history,
                                          minimum_subscriber_cursor_locked());
  oldest_before = corney_ble_frame_history_oldest_id(&event_history);
  if (corney_ble_frame_history_count(&event_history) ==
      event_history.capacity) {
    had_overwritten_frame =
        corney_ble_frame_history_get(&event_history, oldest_before,
                                     &overwritten_frame) == 0;
  }
  err = corney_ble_frame_history_push(&event_history, frame, NULL,
                                      &did_overwrite);
  if (err == 0 && did_overwrite) {
    oldest_after = corney_ble_frame_history_oldest_id(&event_history);
    advance_overwritten_cursors_locked(oldest_after);
  }
  if (overwrote != NULL) {
    *overwrote = did_overwrite;
  }
  if (overwritten_sequence != NULL && did_overwrite && had_overwritten_frame) {
    *overwritten_sequence = frame_sequence(&overwritten_frame);
  }
  return err;
}

int corney_ble_transport_publish(uint8_t type, uint8_t flags,
                                 const uint8_t *payload, size_t payload_len) {
  struct corney_ble_frame frame;
  uint32_t overwritten_sequence = 0U;
  bool overwrote = false;
  int err;
  k_spinlock_key_t key = k_spin_lock(&transport_lock);
  if (corney_ble_subscriber_count(&subscribers) == 0U) {
    err = -ENOTCONN;
  } else {
    err = corney_ble_encode_frame(
        &frame, type, flags, allocate_sequence_locked(), payload, payload_len);
    if (err == 0) {
      err = publish_frame_locked(&frame, &overwrote, &overwritten_sequence);
    }
  }
  k_spin_unlock(&transport_lock, key);
  if (err == 0 && overwrote) {
    record_queue_drop(overwritten_sequence);
  }
  if (err == 0) {
    k_work_reschedule(&transport_work, K_NO_WAIT);
  }
  return err;
}

static void maybe_queue_overflow_report(void) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_DIAGNOSTICS)
  struct corney_ble_frame frame;
  uint32_t overwritten_sequence = 0U;
  bool overwrote = false;
  int err;
  k_spinlock_key_t key;
  if (!atomic_cas(&overflow_report_pending, 1, 0)) {
    return;
  }
  key = k_spin_lock(&transport_lock);
  err = corney_ble_encode_queue_overflow(
      &frame, 0, allocate_sequence_locked(),
      (uint32_t)atomic_get(&queue_drop_count),
      (uint32_t)atomic_get(&last_dropped_sequence));
  if (err == 0) {
    err = publish_frame_locked(&frame, &overwrote, &overwritten_sequence);
  }
  k_spin_unlock(&transport_lock, key);
  if (err != 0) {
    atomic_set(&overflow_report_pending, 1);
  } else if (overwrote) {
    record_queue_drop(overwritten_sequence);
  }
#endif
}

static bool take_candidate(struct transport_candidate *candidate,
                           int64_t now_ms) {
  uint64_t oldest_id;
  uint64_t next_id;
  size_t index;
  k_spinlock_key_t key = k_spin_lock(&transport_lock);

  oldest_id = corney_ble_frame_history_oldest_id(&event_history);
  next_id = corney_ble_frame_history_next_id(&event_history);
  index = corney_ble_subscriber_next_ready(&subscribers, oldest_id, next_id,
                                           now_ms);
  if (index != SIZE_MAX) {
    struct corney_ble_subscriber *subscriber = &subscribers.slots[index];
    candidate->conn = bt_conn_ref((struct bt_conn *)subscriber->connection);
    candidate->slot_index = index;
    candidate->snapshot =
        subscriber->state == CORNEY_BLE_SUBSCRIBER_INITIALIZING;
    candidate->snapshot_sequence = subscriber->snapshot_sequence;
    candidate->entry_id = subscriber->next_entry_id;
    candidate->stream_start_pending = subscriber->stream_start_pending;
    if (!candidate->snapshot &&
        corney_ble_frame_history_get(&event_history, candidate->entry_id,
                                     &candidate->frame) != 0) {
      bt_conn_unref(candidate->conn);
      candidate->conn = NULL;
    } else {
      k_spin_unlock(&transport_lock, key);
      return true;
    }
  }

  k_spin_unlock(&transport_lock, key);
  return false;
}

static int64_t next_work_delay_ms(int64_t now_ms) {
  int64_t delay;
  k_spinlock_key_t key = k_spin_lock(&transport_lock);
  delay = corney_ble_subscriber_next_delay_ms(
      &subscribers, corney_ble_frame_history_oldest_id(&event_history),
      corney_ble_frame_history_next_id(&event_history), now_ms);
  k_spin_unlock(&transport_lock, key);
  return delay;
}

static void schedule_pending_work(void) {
  int64_t delay = next_work_delay_ms(k_uptime_get());
  if (delay >= 0) {
    k_work_reschedule(&transport_work, delay == 0 ? K_NO_WAIT : K_MSEC(delay));
  }
}

static void apply_candidate_result(const struct transport_candidate *candidate,
                                   int result, int64_t now_ms) {
  enum corney_ble_transport_action action;
  struct corney_ble_subscriber *subscriber;
  bool remove_subscriber = false;
  k_spinlock_key_t key = k_spin_lock(&transport_lock);

  subscriber = corney_ble_subscriber_find(&subscribers, candidate->conn);
  if (subscriber == NULL ||
      subscriber != &subscribers.slots[candidate->slot_index]) {
    k_spin_unlock(&transport_lock, key);
    return;
  }
  action =
      corney_ble_transport_action_for_result(result, subscriber->retry_count);

  if (candidate->snapshot) {
    if (subscriber->state != CORNEY_BLE_SUBSCRIBER_INITIALIZING) {
      k_spin_unlock(&transport_lock, key);
      return;
    }
    if (result == 0) {
      subscriber->state = CORNEY_BLE_SUBSCRIBER_ACTIVE;
      subscriber->retry_count = 0U;
      subscriber->retry_at_ms = 0;
    } else if (action == CORNEY_BLE_TRANSPORT_RETRY) {
      subscriber->retry_count++;
      subscriber->retry_at_ms = now_ms + (int64_t)CORNEY_BLE_TRANSPORT_RETRY_MS;
    } else {
      atomic_inc(&transport_drop_count);
      remove_subscriber = true;
    }
  } else if (subscriber->state == CORNEY_BLE_SUBSCRIBER_ACTIVE &&
             subscriber->next_entry_id == candidate->entry_id) {
    if (action == CORNEY_BLE_TRANSPORT_RETRY) {
      subscriber->retry_count++;
      subscriber->retry_at_ms = now_ms + (int64_t)CORNEY_BLE_TRANSPORT_RETRY_MS;
    } else {
      subscriber->next_entry_id++;
      subscriber->retry_count = 0U;
      subscriber->retry_at_ms = 0;
      subscriber->stream_start_pending =
          corney_ble_stream_start_pending_after_result(
              subscriber->stream_start_pending, result);
      if (result != 0) {
        atomic_inc(&transport_drop_count);
      }
      if (result == -ENOTCONN) {
        remove_subscriber = true;
      }
    }
  }
  k_spin_unlock(&transport_lock, key);

  if (remove_subscriber) {
    corney_ble_transport_unsubscribe(candidate->conn);
  }
}

static void transport_work_handler(struct k_work *work) {
  struct transport_candidate candidate = {0};
  int64_t now_ms = k_uptime_get();
  int err;
  ARG_UNUSED(work);

  if (!take_candidate(&candidate, now_ms)) {
    schedule_pending_work();
    return;
  }

  if (candidate.snapshot) {
    err = corney_ble_encode_initial_snapshot(&candidate.frame,
                                             candidate.snapshot_sequence);
  } else {
    corney_ble_apply_stream_start(&candidate.frame,
                                  candidate.stream_start_pending);
    err = 0;
  }
  if (err == 0) {
    err = corney_gatt_notify_event(candidate.conn, &candidate.frame);
  }
  apply_candidate_result(&candidate, err, now_ms);
  bt_conn_unref(candidate.conn);

  if (err == 0) {
    maybe_queue_overflow_report();
  }
  schedule_pending_work();
}
