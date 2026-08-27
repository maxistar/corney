#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

LOG_MODULE_REGISTER(zmk_gatt_layer_exposition, LOG_LEVEL_INF);

#define LAYER_NOTIFY_DELAY_MS 5
#define LAYER_NOTIFY_RETRY_MS 10

static atomic_t current_layer = ATOMIC_INIT(0);
static atomic_t notify_enabled = ATOMIC_INIT(0);
static atomic_t pending_layer = ATOMIC_INIT(0);

static struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x12341234, 0x1234, 0x5678, 0x7856, 0x123412345678));

static struct bt_uuid_128 layer_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x12341234, 0x1234, 0x5678, 0x7856, 0x123412345679));

static int32_t highest_active_layer(void) {
  return (int32_t)zmk_keymap_highest_layer_active();
}

static ssize_t read_layer(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset) {
  int32_t le_layer = sys_cpu_to_le32((int32_t)atomic_get(&current_layer));

  ARG_UNUSED(attr);

  return bt_gatt_attr_read(conn, attr, buf, len, offset, &le_layer,
                           sizeof(le_layer));
}

static int notify_current_layer(void);
static void schedule_layer_notification(void);
static void notify_layer_work_handler(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(layer_notify_work, notify_layer_work_handler);

static void apply_pending_layer(struct k_work *work) {
  int32_t layer = (int32_t)atomic_get(&pending_layer);
  int err;

  ARG_UNUSED(work);

  err = zmk_keymap_layer_to((zmk_keymap_layer_id_t)layer);

  if (err) {
    LOG_WRN("Failed to select requested layer %d (%d)", layer, err);
  }

  /*
   * Force a delayed final notification even when the requested layer was
   * already active and ZMK did not raise an event.
   */
  schedule_layer_notification();
}

K_WORK_DEFINE(layer_command_work, apply_pending_layer);

static ssize_t write_layer(struct bt_conn *conn,
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

static void layer_ccc_cfg_changed(const struct bt_gatt_attr *attr,
                                  uint16_t value) {
  ARG_UNUSED(attr);

  atomic_set(&notify_enabled, value == BT_GATT_CCC_NOTIFY);
}

BT_GATT_SERVICE_DEFINE(
    gatt_layer_exposition_svc, BT_GATT_PRIMARY_SERVICE(&service_uuid),
    BT_GATT_CHARACTERISTIC(&layer_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |
                               BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT,
                           read_layer, write_layer, NULL),
    BT_GATT_CCC(layer_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE));

static int notify_current_layer(void) {
  int32_t le_layer;
  int err;

  if (!atomic_get(&notify_enabled)) {
    return 0;
  }

  le_layer = sys_cpu_to_le32((int32_t)atomic_get(&current_layer));
  err = bt_gatt_notify(NULL, &gatt_layer_exposition_svc.attrs[2], &le_layer,
                       sizeof(le_layer));
  if (err) {
    LOG_WRN("Failed to notify layer update (%d)", err);
  }

  return err;
}

static void notify_layer_work_handler(struct k_work *work) {
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

  if (!atomic_get(&notify_enabled)) {
    return;
  }

  err = k_work_reschedule(&layer_notify_work, K_MSEC(LAYER_NOTIFY_DELAY_MS));
  if (err < 0) {
    LOG_WRN("Failed to schedule layer notification (%d)", err);
  }
}

static int set_current_layer(int32_t layer) {
  atomic_val_t previous_layer = atomic_set(&current_layer, layer);

  if (previous_layer == layer) {
    return 0;
  }

  schedule_layer_notification();
  return 0;
}

static int gatt_layer_exposition_listener(const zmk_event_t *eh) {
  ARG_UNUSED(eh);

  return set_current_layer(highest_active_layer());
}

ZMK_LISTENER(gatt_layer_exposition, gatt_layer_exposition_listener);
ZMK_SUBSCRIPTION(gatt_layer_exposition, zmk_layer_state_changed);

static int zmk_gatt_layer_exposition_init(void) {
  atomic_set(&current_layer, highest_active_layer());
  return 0;
}

SYS_INIT(zmk_gatt_layer_exposition_init, APPLICATION,
         CONFIG_APPLICATION_INIT_PRIORITY);
