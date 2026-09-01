#include <errno.h>

#include <zephyr/kernel.h>
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EVENT_TEST_LOG)
#include <zephyr/sys/printk.h>
#endif

#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>

#include <corney/ble_contract.h>
#include <corney/ble_events.h>
#include <corney/ble_transport.h>
#include <corney/layer_state.h>

#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_KEY_EVENTS) ||                       \
    IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_COMBO_EVENTS) ||                     \
    IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_LAYER_EVENTS)
static int publish_encoded(struct corney_ble_frame *frame) {
  /* The transport allocates the authoritative stream sequence. */
  uint8_t type = frame->data[1];
  uint8_t flags = frame->data[2];
  uint8_t payload_len = frame->data[3];
  return corney_ble_transport_publish(
      type, flags, &frame->data[CORNEY_BLE_FRAME_HEADER_SIZE], payload_len);
}
#endif

void corney_ble_publish_key(bool pressed, uint8_t position, uint8_t layer) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_KEY_EVENTS)
  struct corney_ble_frame frame;
  if (corney_ble_encode_key(&frame, 0, 0, pressed, position, layer) == 0) {
    publish_encoded(&frame);
  }
#else
  ARG_UNUSED(pressed);
  ARG_UNUSED(position);
  ARG_UNUSED(layer);
#endif
}

void corney_ble_publish_combo(uint16_t combo_id, bool pressed, uint8_t layer,
                              const uint8_t *positions, size_t position_count) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_COMBO_EVENTS)
  struct corney_ble_frame frame;
  if (corney_ble_encode_combo(&frame, 0, 0, combo_id, pressed, layer, positions,
                              position_count) == 0) {
    publish_encoded(&frame);
  }
#else
  ARG_UNUSED(combo_id);
  ARG_UNUSED(pressed);
  ARG_UNUSED(layer);
  ARG_UNUSED(positions);
  ARG_UNUSED(position_count);
#endif
}

void corney_ble_publish_layer(uint8_t layer, uint8_t previous_layer,
                              uint8_t cause, uint8_t origin_position,
                              uint8_t flags) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_LAYER_EVENTS)
  struct corney_ble_frame frame;
  if (corney_ble_encode_layer(&frame, flags, 0, layer, previous_layer, cause,
                              origin_position) == 0) {
    publish_encoded(&frame);
  }
#else
  ARG_UNUSED(layer);
  ARG_UNUSED(previous_layer);
  ARG_UNUSED(cause);
  ARG_UNUSED(origin_position);
  ARG_UNUSED(flags);
#endif
}

int corney_ble_encode_initial_snapshot(struct corney_ble_frame *frame,
                                       uint32_t sequence) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_LAYER_EVENTS)
  uint8_t layer = corney_layer_current();
  return corney_ble_encode_layer(
      frame, CORNEY_BLE_FLAG_STREAM_START | CORNEY_BLE_FLAG_SNAPSHOT, sequence,
      layer, layer, CORNEY_BLE_LAYER_CAUSE_RESTORE, 0xff);
#else
  ARG_UNUSED(frame);
  ARG_UNUSED(sequence);
  return -ENOTSUP;
#endif
}

static int position_listener(const zmk_event_t *eh) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_KEY_EVENTS) ||                       \
    IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EVENT_TEST_LOG)
  const struct zmk_position_state_changed *ev =
      as_zmk_position_state_changed(eh);
  if (ev != NULL && ev->position <= CORNEY_BLE_MAX_POSITION) {
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_KEY_EVENTS)
    corney_ble_publish_key(ev->state, (uint8_t)ev->position,
                           (uint8_t)zmk_keymap_highest_layer_active());
#endif
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_EVENT_TEST_LOG)
    printk("key_telemetry: position=%u action=%s layer=%u\n", ev->position,
           ev->state ? "pressed" : "released",
           zmk_keymap_highest_layer_active());
#endif
  }
#else
  ARG_UNUSED(eh);
#endif
  return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(corney_ble_positions, position_listener);
ZMK_SUBSCRIPTION(corney_ble_positions, zmk_position_state_changed);
