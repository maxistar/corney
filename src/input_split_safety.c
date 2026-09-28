/*
 * Copyright (c) 2026 The Corney Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>

#if IS_ENABLED(CONFIG_BT)
#include <zephyr/bluetooth/conn.h>
#endif

#include <corney/input_split_safety.h>
#include <zmk/pointing/input_split.h>

#define DT_DRV_COMPAT zmk_input_split

#define CORNEY_TRACKED_BUTTON_COUNT 3U

struct corney_split_proxy_state {
  uint8_t reg;
  atomic_t active_buttons;
};

#define CORNEY_SPLIT_PROXY_STATE(inst)                                         \
  {.reg = DT_INST_REG_ADDR(inst), .active_buttons = ATOMIC_INIT(0)},

static struct corney_split_proxy_state proxy_states[] = {
    DT_INST_FOREACH_STATUS_OKAY(CORNEY_SPLIT_PROXY_STATE)};

BUILD_ASSERT(ARRAY_SIZE(proxy_states) > 0,
             "Disconnect safety requires an input-split proxy");

static void track_split_button(size_t proxy_index, struct input_event *event) {
  if (event->type != INPUT_EV_KEY || event->code < INPUT_BTN_0 ||
      event->code >= INPUT_BTN_0 + CORNEY_TRACKED_BUTTON_COUNT) {
    return;
  }

  const uint8_t index = event->code - INPUT_BTN_0;
  if (event->value != 0) {
    atomic_set_bit(&proxy_states[proxy_index].active_buttons, index);
  } else {
    atomic_clear_bit(&proxy_states[proxy_index].active_buttons, index);
  }
}

#define CORNEY_DEFINE_SPLIT_PROXY_TRACKER(inst)                                \
  static void track_split_button_##inst(struct input_event *event) {           \
    track_split_button(inst, event);                                           \
  }                                                                            \
  INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_DRV_INST(inst)),                      \
                        track_split_button_##inst);

DT_INST_FOREACH_STATUS_OKAY(CORNEY_DEFINE_SPLIT_PROXY_TRACKER)

int corney_input_split_release_buttons(void) {
  atomic_val_t pending[ARRAY_SIZE(proxy_states)];
  size_t release_count = 0U;
  int first_error = 0;

  for (size_t proxy_index = 0U; proxy_index < ARRAY_SIZE(proxy_states);
       proxy_index++) {
    pending[proxy_index] =
        atomic_set(&proxy_states[proxy_index].active_buttons, 0);
    for (uint8_t button_index = 0U; button_index < CORNEY_TRACKED_BUTTON_COUNT;
         button_index++) {
      if ((pending[proxy_index] & BIT(button_index)) != 0) {
        release_count++;
      }
    }
  }

  for (size_t proxy_index = 0U; proxy_index < ARRAY_SIZE(proxy_states);
       proxy_index++) {
    for (uint8_t button_index = 0U; button_index < CORNEY_TRACKED_BUTTON_COUNT;
         button_index++) {
      if ((pending[proxy_index] & BIT(button_index)) == 0) {
        continue;
      }

      release_count--;
      int err = zmk_input_split_report_peripheral_event(
          proxy_states[proxy_index].reg, INPUT_EV_KEY,
          INPUT_BTN_0 + button_index, 0, release_count == 0U);
      if (err != 0 && first_error == 0) {
        first_error = err;
      }
    }
  }

  return first_error;
}

#if IS_ENABLED(CONFIG_BT)
static void release_buttons_work_handler(struct k_work *work) {
  ARG_UNUSED(work);
  corney_input_split_release_buttons();
}

K_WORK_DEFINE(release_buttons_work, release_buttons_work_handler);

static void split_connection_disconnected(struct bt_conn *connection,
                                          uint8_t reason) {
  struct bt_conn_info info;

  ARG_UNUSED(reason);
  if (bt_conn_get_info(connection, &info) != 0 ||
      info.role != BT_CONN_ROLE_CENTRAL) {
    return;
  }

  k_work_submit(&release_buttons_work);
}

BT_CONN_CB_DEFINE(corney_split_pointing_connection_callbacks) = {
    .disconnected = split_connection_disconnected,
};
#endif
