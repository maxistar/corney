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

#define CORNEY_POINTING_INPUT_SPLIT_REG DT_INST_REG_ADDR(0)
#define CORNEY_TRACKED_BUTTON_COUNT 3U

static atomic_t active_buttons;

/* This callback is registered only for the zmk,input-split proxy. Local
 * pointing devices use a different compatible and never enter this state. */
static void track_split_button(struct input_event *event) {
  if (event->type != INPUT_EV_KEY || event->code < INPUT_BTN_0 ||
      event->code >= INPUT_BTN_0 + CORNEY_TRACKED_BUTTON_COUNT) {
    return;
  }

  const uint8_t index = event->code - INPUT_BTN_0;
  if (event->value != 0) {
    atomic_set_bit(&active_buttons, index);
  } else {
    atomic_clear_bit(&active_buttons, index);
  }
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_DRV_INST(0)), track_split_button);

int corney_input_split_release_buttons(void) {
  atomic_val_t pending = atomic_set(&active_buttons, 0);
  int first_error = 0;

  for (uint8_t index = 0U; index < CORNEY_TRACKED_BUTTON_COUNT; index++) {
    if ((pending & BIT(index)) == 0) {
      continue;
    }

    pending &= ~BIT(index);
    int err = zmk_input_split_report_peripheral_event(
        CORNEY_POINTING_INPUT_SPLIT_REG, INPUT_EV_KEY, INPUT_BTN_0 + index, 0,
        pending == 0);
    if (err != 0 && first_error == 0) {
      first_error = err;
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
