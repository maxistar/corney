#include <errno.h>
#include <stdint.h>

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <corney/ble_contract.h>
#include <corney/ble_events.h>
#include <corney/ble_transport.h>
#include <corney/gatt_service.h>
#include <corney/layer_state.h>

LOG_MODULE_REGISTER(corney_gatt_service, LOG_LEVEL_INF);

#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION)
static void subscription_start_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(subscription_start_work,
                        subscription_start_work_handler);
#endif

static struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xb34a0001, 0xe782, 0x4706, 0x8f9c, 0x6c056c416507));
static struct bt_uuid_128 layer_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xb34a0002, 0xe782, 0x4706, 0x8f9c, 0x6c056c416507));
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION)
static struct bt_uuid_128 capabilities_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xb34a0003, 0xe782, 0x4706, 0x8f9c, 0x6c056c416507));
static struct bt_uuid_128 event_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0xb34a0004, 0xe782, 0x4706, 0x8f9c, 0x6c056c416507));
#endif

#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION)
static uint16_t capability_flags(void) {
  uint16_t flags =
      CORNEY_BLE_CAP_LEGACY_LAYER_REGISTER | CORNEY_BLE_CAP_LEGACY_LAYER_WRITE;
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_KEY_EVENTS)
  flags |= CORNEY_BLE_CAP_KEY_EVENTS;
#endif
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_COMBO_EVENTS)
  flags |= CORNEY_BLE_CAP_COMBO_EVENTS;
#endif
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_LAYER_EVENTS)
  flags |= CORNEY_BLE_CAP_LAYER_EVENTS;
#endif
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_DIAGNOSTICS)
  flags |= CORNEY_BLE_CAP_DIAGNOSTICS;
#endif
  return flags;
}

static ssize_t capabilities_read(struct bt_conn *conn,
                                 const struct bt_gatt_attr *attr, void *buf,
                                 uint16_t len, uint16_t offset) {
  uint8_t capabilities[CORNEY_BLE_CAPABILITIES_SIZE];
  corney_ble_encode_capabilities(capability_flags(), capabilities);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, capabilities,
                           sizeof(capabilities));
}

static void event_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value) {
  ARG_UNUSED(attr);
  ARG_UNUSED(value);
}

static ssize_t event_ccc_write(struct bt_conn *conn,
                               const struct bt_gatt_attr *attr,
                               uint16_t value) {
  int err;
  ARG_UNUSED(attr);

  if (value == 0U) {
    corney_ble_transport_release(conn);
    return sizeof(value);
  }
  if (value != BT_GATT_CCC_NOTIFY) {
    return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
  }

  err = corney_ble_transport_claim(conn);
  if (err == -EBUSY) {
    return BT_GATT_ERR(BT_ATT_ERR_INSUFFICIENT_RESOURCES);
  }
  if (err != 0) {
    return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
  }

  corney_ble_transport_start_epoch();
  k_work_reschedule(&subscription_start_work, K_MSEC(1));
  return sizeof(value);
}

static bool event_ccc_match(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr) {
  ARG_UNUSED(attr);
  return corney_ble_transport_is_owner(conn);
}

static struct _bt_gatt_ccc event_ccc = BT_GATT_CCC_INITIALIZER(
    event_ccc_changed, event_ccc_write, event_ccc_match);

static void subscription_start_work_handler(struct k_work *work) {
  struct bt_conn *conn;
  ARG_UNUSED(work);

  conn = corney_ble_transport_owner();
  if (conn == NULL) {
    return;
  }
  if (bt_gatt_is_subscribed(conn, corney_gatt_event_attr(),
                            BT_GATT_CCC_NOTIFY)) {
    corney_ble_publish_initial_snapshots();
    corney_ble_transport_finish_epoch();
  } else {
    corney_ble_transport_release(conn);
  }
  bt_conn_unref(conn);
}
#endif

#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION)
BT_GATT_SERVICE_DEFINE(
    corney_gatt_svc, BT_GATT_PRIMARY_SERVICE(&service_uuid),
    BT_GATT_CHARACTERISTIC(&layer_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |
                               BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT,
                           corney_layer_read, corney_layer_write, NULL),
    BT_GATT_CCC(corney_layer_ccc_changed,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&capabilities_uuid.uuid, BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ, capabilities_read, NULL, NULL),
    BT_GATT_CHARACTERISTIC(&event_uuid.uuid, BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_NONE, NULL, NULL, NULL),
    BT_GATT_CCC_MANAGED(&event_ccc, BT_GATT_PERM_READ_ENCRYPT |
                                        BT_GATT_PERM_WRITE_ENCRYPT));
#else
BT_GATT_SERVICE_DEFINE(
    corney_gatt_svc, BT_GATT_PRIMARY_SERVICE(&service_uuid),
    BT_GATT_CHARACTERISTIC(&layer_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |
                               BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT,
                           corney_layer_read, corney_layer_write, NULL),
    BT_GATT_CCC(corney_layer_ccc_changed,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE));
#endif

int corney_gatt_notify_legacy_layer(int32_t layer) {
  int32_t le_layer = sys_cpu_to_le32(layer);
  return bt_gatt_notify(NULL, &corney_gatt_svc.attrs[2], &le_layer,
                        sizeof(le_layer));
}

const struct bt_gatt_attr *corney_gatt_event_attr(void) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION)
  return &corney_gatt_svc.attrs[7];
#else
  return NULL;
#endif
}

int corney_gatt_notify_event(struct bt_conn *conn,
                             const struct corney_ble_frame *frame) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION)
  if (conn == NULL || frame == NULL || !corney_ble_transport_is_owner(conn) ||
      !bt_gatt_is_subscribed(conn, corney_gatt_event_attr(),
                             BT_GATT_CCC_NOTIFY)) {
    return -ENOTCONN;
  }
  return bt_gatt_notify(conn, corney_gatt_event_attr(), frame->data,
                        frame->len);
#else
  ARG_UNUSED(conn);
  ARG_UNUSED(frame);
  return -ENOTSUP;
#endif
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
  ARG_UNUSED(reason);
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION)
  corney_ble_transport_release(conn);
#else
  ARG_UNUSED(conn);
#endif
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
                             enum bt_security_err err) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION)
  if (err == BT_SECURITY_ERR_SUCCESS && level >= BT_SECURITY_L2 &&
      bt_gatt_is_subscribed(conn, corney_gatt_event_attr(),
                            BT_GATT_CCC_NOTIFY) &&
      corney_ble_transport_claim(conn) == 0) {
    corney_ble_transport_start_epoch();
    k_work_reschedule(&subscription_start_work, K_MSEC(1));
  }
#else
  ARG_UNUSED(conn);
  ARG_UNUSED(level);
  ARG_UNUSED(err);
#endif
}

BT_CONN_CB_DEFINE(corney_gatt_conn_callbacks) = {
    .disconnected = disconnected,
    .security_changed = security_changed,
};
