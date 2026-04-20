#include <stdbool.h>
#include <stdint.h>

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

LOG_MODULE_REGISTER(zmk_gatt_layer_exposition, LOG_LEVEL_INF);

static int32_t current_layer;
static bool notify_enabled;

static struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x12341234, 0x1234, 0x5678, 0x7856, 0x123412345678));

static struct bt_uuid_128 layer_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x12341234, 0x1234, 0x5678, 0x7856, 0x123412345679));

static int32_t highest_active_layer(void) { return (int32_t)zmk_keymap_highest_layer_active(); }

static ssize_t read_layer(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf,
                          uint16_t len,
                          uint16_t offset) {
    int32_t le_layer = sys_cpu_to_le32(current_layer);

    ARG_UNUSED(attr);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &le_layer, sizeof(le_layer));
}

static void layer_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value) {
    ARG_UNUSED(attr);

    notify_enabled = (value == BT_GATT_CCC_NOTIFY);
}

BT_GATT_SERVICE_DEFINE(gatt_layer_exposition_svc,
                       BT_GATT_PRIMARY_SERVICE(&service_uuid),
                       BT_GATT_CHARACTERISTIC(&layer_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                                              BT_GATT_PERM_READ,
                                              read_layer,
                                              NULL,
                                              NULL),
                       BT_GATT_CCC(layer_ccc_cfg_changed,
                                   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE));

static int set_current_layer(int32_t layer) {
    int32_t le_layer;
    int err;

    if (current_layer == layer) {
        return 0;
    }

    current_layer = layer;

    if (!notify_enabled) {
        return 0;
    }

    le_layer = sys_cpu_to_le32(current_layer);
    err = bt_gatt_notify(NULL, &gatt_layer_exposition_svc.attrs[2], &le_layer, sizeof(le_layer));
    if (err) {
        LOG_WRN("Failed to notify layer update (%d)", err);
    }

    return err;
}

static int gatt_layer_exposition_listener(const zmk_event_t *eh) {
    ARG_UNUSED(eh);

    return set_current_layer(highest_active_layer());
}

ZMK_LISTENER(gatt_layer_exposition, gatt_layer_exposition_listener);
ZMK_SUBSCRIPTION(gatt_layer_exposition, zmk_layer_state_changed);

static int zmk_gatt_layer_exposition_init(void) {
    current_layer = highest_active_layer();
    return 0;
}

SYS_INIT(zmk_gatt_layer_exposition_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
