#include <errno.h>
#include <stdint.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#include <corney/ble_contract.h>
#include <corney/ble_events.h>
#include <corney/gatt_service.h>
#include <corney/layer_state.h>

LOG_MODULE_REGISTER(corney_layer_state, LOG_LEVEL_INF);

#define LAYER_NOTIFY_DELAY_MS 5
#define LAYER_NOTIFY_RETRY_MS 10

static atomic_t current_layer = ATOMIC_INIT(0);
static atomic_t pending_layer = ATOMIC_INIT(0);
static atomic_t legacy_notify_enabled = ATOMIC_INIT(0);
static atomic_t remote_write_pending = ATOMIC_INIT(0);

static void layer_notify_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(layer_notify_work, layer_notify_work_handler);

static uint8_t highest_active_layer(void) {
  return (uint8_t)zmk_keymap_highest_layer_active();
}

uint8_t corney_layer_current(void) {
  return (uint8_t)atomic_get(&current_layer);
}

static int notify_current_layer(void) {
  if (!atomic_get(&legacy_notify_enabled)) {
    return 0;
  }
  return corney_gatt_notify_legacy_layer((int32_t)atomic_get(&current_layer));
}

static void layer_notify_work_handler(struct k_work *work) {
  int err;

  ARG_UNUSED(work);
  err = notify_current_layer();
  if (err == -ENOMEM || err == -EAGAIN) {
    err = k_work_reschedule(&layer_notify_work, K_MSEC(LAYER_NOTIFY_RETRY_MS));
    if (err < 0) {
      LOG_WRN("Failed to reschedule layer notification (%d)", err);
    }
  }
}

static void schedule_layer_notification(void) {
  int err;

  if (!atomic_get(&legacy_notify_enabled)) {
    return;
  }
  err = k_work_reschedule(&layer_notify_work, K_MSEC(LAYER_NOTIFY_DELAY_MS));
  if (err < 0) {
    LOG_WRN("Failed to schedule layer notification (%d)", err);
  }
}

static int set_current_layer(uint8_t layer, uint8_t cause) {
  uint8_t previous = (uint8_t)atomic_set(&current_layer, layer);

  if (previous == layer) {
    return 0;
  }
  schedule_layer_notification();
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_LAYER_EVENTS)
  corney_ble_publish_layer(layer, previous, cause, 0xff, 0);
#else
  ARG_UNUSED(cause);
#endif
  return 0;
}

static void apply_pending_layer(struct k_work *work) {
  uint8_t layer = (uint8_t)atomic_get(&pending_layer);
  int err;

  ARG_UNUSED(work);
  atomic_set(&remote_write_pending, 1);
  err = zmk_keymap_layer_to((zmk_keymap_layer_id_t)layer);
  if (err) {
    LOG_WRN("Failed to select requested layer %d (%d)", layer, err);
  }
  set_current_layer(highest_active_layer(),
                    CORNEY_BLE_LAYER_CAUSE_REMOTE_WRITE);
  atomic_set(&remote_write_pending, 0);
  schedule_layer_notification();
}

K_WORK_DEFINE(layer_command_work, apply_pending_layer);

ssize_t corney_layer_read(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset) {
  int32_t le_layer = sys_cpu_to_le32((int32_t)atomic_get(&current_layer));
  ARG_UNUSED(attr);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &le_layer,
                           sizeof(le_layer));
}

ssize_t corney_layer_write(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr, const void *buf,
                           uint16_t len, uint16_t offset, uint8_t flags) {
  int32_t layer;
  int err;

  ARG_UNUSED(conn);
  ARG_UNUSED(attr);

  if (flags != 0) {
    return BT_GATT_ERR(BT_ATT_ERR_WRITE_REQ_REJECTED);
  }
  if (offset != 0) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }
  if (len != sizeof(layer)) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
  }
  layer = (int32_t)sys_get_le32(buf);
  if (layer < 0 || layer >= ZMK_KEYMAP_LAYERS_LEN) {
    return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
  }

  atomic_set(&pending_layer, (atomic_val_t)layer);
  err = k_work_submit(&layer_command_work);
  if (err < 0) {
    LOG_WRN("Failed to schedule requested layer %d (%d)", layer, err);
    return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
  }
  return len;
}

void corney_layer_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value) {
  ARG_UNUSED(attr);
  atomic_set(&legacy_notify_enabled, value == BT_GATT_CCC_NOTIFY);
}

static int layer_listener(const zmk_event_t *eh) {
  ARG_UNUSED(eh);
  return set_current_layer(highest_active_layer(),
                           atomic_get(&remote_write_pending)
                               ? CORNEY_BLE_LAYER_CAUSE_REMOTE_WRITE
                               : CORNEY_BLE_LAYER_CAUSE_PHYSICAL);
}

ZMK_LISTENER(corney_layer, layer_listener);
ZMK_SUBSCRIPTION(corney_layer, zmk_layer_state_changed);

static int layer_state_init(void) {
  atomic_set(&current_layer, highest_active_layer());
  return 0;
}

SYS_INIT(layer_state_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
