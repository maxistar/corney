#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

#include <corney/ble_contract.h>
#include <corney/ble_frame_queue.h>
#include <corney/ble_subscription_owner.h>
#include <corney/ble_transport.h>
#include <corney/ble_transport_policy.h>
#include <corney/gatt_service.h>

LOG_MODULE_REGISTER(corney_ble_transport, LOG_LEVEL_INF);

static struct corney_ble_frame
    queue_storage[CONFIG_ZMK_KEYBOARD_HELPER_EVENT_QUEUE_SIZE];
static struct corney_ble_frame_queue event_queue = {
    .storage = queue_storage,
    .capacity = CONFIG_ZMK_KEYBOARD_HELPER_EVENT_QUEUE_SIZE,
};
static struct corney_ble_subscription_owner subscription_owner;
static struct k_spinlock owner_lock;
static struct k_spinlock queue_lock;
static atomic_t next_sequence = ATOMIC_INIT(CORNEY_BLE_STREAM_INITIAL_SEQUENCE);
static atomic_t queue_drop_count = ATOMIC_INIT(0);
static atomic_t transport_drop_count = ATOMIC_INIT(0);
static atomic_t last_dropped_sequence = ATOMIC_INIT(0);
static atomic_t overflow_report_pending = ATOMIC_INIT(0);
static atomic_t stream_start_pending = ATOMIC_INIT(0);
static atomic_t epoch_initializing = ATOMIC_INIT(0);
static uint8_t retry_count;

static void transport_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(transport_work, transport_work_handler);

static uint32_t allocate_sequence(void) {
  return (uint32_t)atomic_inc(&next_sequence);
}

struct bt_conn *corney_ble_transport_owner(void) {
  struct bt_conn *conn = NULL;
  k_spinlock_key_t key = k_spin_lock(&owner_lock);

  if (subscription_owner.connection != NULL) {
    conn = bt_conn_ref((struct bt_conn *)subscription_owner.connection);
  }
  k_spin_unlock(&owner_lock, key);
  return conn;
}

bool corney_ble_transport_is_owner(struct bt_conn *conn) {
  bool is_owner;
  k_spinlock_key_t key = k_spin_lock(&owner_lock);
  is_owner = corney_ble_subscription_is_owner(&subscription_owner, conn);
  k_spin_unlock(&owner_lock, key);
  return is_owner;
}

int corney_ble_transport_claim(struct bt_conn *conn) {
  int err = 0;
  bool new_owner;
  struct bt_conn *held;
  k_spinlock_key_t key;

  if (conn == NULL) {
    return -EINVAL;
  }

  held = bt_conn_ref(conn);
  key = k_spin_lock(&owner_lock);
  new_owner = subscription_owner.connection == NULL;
  err = corney_ble_subscription_claim(&subscription_owner, conn);
  k_spin_unlock(&owner_lock, key);

  if (err != 0 || !new_owner) {
    bt_conn_unref(held);
  }

  return err;
}

void corney_ble_transport_release(struct bt_conn *conn) {
  bool released;
  k_spinlock_key_t key = k_spin_lock(&owner_lock);
  released = corney_ble_subscription_release(&subscription_owner, conn);
  k_spin_unlock(&owner_lock, key);

  if (released) {
    bt_conn_unref(conn);
    k_work_cancel_delayable(&transport_work);
    key = k_spin_lock(&queue_lock);
    corney_ble_frame_queue_purge(&event_queue);
    k_spin_unlock(&queue_lock, key);
    retry_count = 0;
    atomic_set(&stream_start_pending, 0);
    atomic_set(&epoch_initializing, 0);
  }
}

void corney_ble_transport_start_epoch(void) {
  k_spinlock_key_t key;
  k_work_cancel_delayable(&transport_work);
  key = k_spin_lock(&queue_lock);
  corney_ble_frame_queue_purge(&event_queue);
  k_spin_unlock(&queue_lock, key);
  atomic_set(&next_sequence, CORNEY_BLE_STREAM_INITIAL_SEQUENCE);
  atomic_set(&queue_drop_count, 0);
  atomic_set(&transport_drop_count, 0);
  atomic_set(&last_dropped_sequence, 0);
  atomic_set(&overflow_report_pending, 0);
  atomic_set(&stream_start_pending, 1);
  atomic_set(&epoch_initializing, 1);
  retry_count = 0;
}

void corney_ble_transport_finish_epoch(void) {
  size_t queued;
  k_spinlock_key_t key;

  atomic_set(&epoch_initializing, 0);
  key = k_spin_lock(&queue_lock);
  queued = corney_ble_frame_queue_count(&event_queue);
  k_spin_unlock(&queue_lock, key);
  if (queued > 0U) {
    k_work_reschedule(&transport_work, K_NO_WAIT);
  }
}

uint32_t corney_ble_transport_drop_count(void) {
  return (uint32_t)(atomic_get(&queue_drop_count) +
                    atomic_get(&transport_drop_count));
}

static void record_queue_drop(uint32_t sequence) {
  atomic_inc(&queue_drop_count);
  atomic_set(&last_dropped_sequence, (atomic_val_t)sequence);
  atomic_set(&overflow_report_pending, 1);
}

int corney_ble_transport_publish_frame(struct corney_ble_frame *frame) {
  struct bt_conn *conn;
  k_spinlock_key_t key;
  int err;

  if (frame == NULL || corney_ble_validate_frame(frame) != 0) {
    return -EINVAL;
  }
  if (atomic_get(&epoch_initializing) &&
      (frame->data[2] & CORNEY_BLE_FLAG_SNAPSHOT) == 0U) {
    return -EAGAIN;
  }
  conn = corney_ble_transport_owner();
  if (conn == NULL) {
    return -ENOTCONN;
  }
  bt_conn_unref(conn);

  key = k_spin_lock(&queue_lock);
  err = corney_ble_frame_queue_push(&event_queue, frame);
  k_spin_unlock(&queue_lock, key);
  if (err != 0) {
    uint32_t sequence =
        (uint32_t)frame->data[4] | ((uint32_t)frame->data[5] << 8) |
        ((uint32_t)frame->data[6] << 16) | ((uint32_t)frame->data[7] << 24);
    record_queue_drop(sequence);
    return err;
  }

  if (!atomic_get(&epoch_initializing)) {
    k_work_reschedule(&transport_work, K_NO_WAIT);
  }
  return 0;
}

int corney_ble_transport_publish(uint8_t type, uint8_t flags,
                                 const uint8_t *payload, size_t payload_len) {
  struct corney_ble_frame frame;
  int err = corney_ble_encode_frame(&frame, type, flags, allocate_sequence(),
                                    payload, payload_len);
  if (err != 0) {
    return err;
  }
  return corney_ble_transport_publish_frame(&frame);
}

static void maybe_queue_overflow_report(void) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_DIAGNOSTICS)
  struct corney_ble_frame frame;
  k_spinlock_key_t key;

  if (!atomic_cas(&overflow_report_pending, 1, 0)) {
    return;
  }
  corney_ble_encode_queue_overflow(
      &frame, 0, allocate_sequence(), (uint32_t)atomic_get(&queue_drop_count),
      (uint32_t)atomic_get(&last_dropped_sequence));
  key = k_spin_lock(&queue_lock);
  int err = corney_ble_frame_queue_push(&event_queue, &frame);
  k_spin_unlock(&queue_lock, key);
  if (err != 0) {
    atomic_set(&overflow_report_pending, 1);
  }
#endif
}

static void transport_work_handler(struct k_work *work) {
  struct corney_ble_frame frame;
  struct bt_conn *conn;
  k_spinlock_key_t key;
  int err;

  ARG_UNUSED(work);

  key = k_spin_lock(&queue_lock);
  err = corney_ble_frame_queue_peek(&event_queue, &frame);
  k_spin_unlock(&queue_lock, key);
  if (err != 0) {
    return;
  }

  conn = corney_ble_transport_owner();
  if (conn == NULL) {
    key = k_spin_lock(&queue_lock);
    corney_ble_frame_queue_pop(&event_queue, &frame);
    k_spin_unlock(&queue_lock, key);
    return;
  }

  bool start_pending = atomic_get(&stream_start_pending);
  corney_ble_apply_stream_start(&frame, start_pending);
  err = corney_gatt_notify_event(conn, &frame);
  bt_conn_unref(conn);

  if (corney_ble_transport_action_for_result(err, retry_count) ==
      CORNEY_BLE_TRANSPORT_RETRY) {
    retry_count++;
    k_work_reschedule(&transport_work, K_MSEC(CORNEY_BLE_TRANSPORT_RETRY_MS));
    return;
  }

  key = k_spin_lock(&queue_lock);
  corney_ble_frame_queue_pop(&event_queue, &frame);
  k_spin_unlock(&queue_lock, key);
  retry_count = 0;
  atomic_set(&stream_start_pending,
             corney_ble_stream_start_pending_after_result(start_pending, err));
  if (err != 0) {
    atomic_inc(&transport_drop_count);
  } else {
    maybe_queue_overflow_report();
  }

  key = k_spin_lock(&queue_lock);
  size_t queued = corney_ble_frame_queue_count(&event_queue);
  k_spin_unlock(&queue_lock, key);
  if (queued > 0U) {
    k_work_reschedule(&transport_work, K_NO_WAIT);
  }
}
