/*
 * Copyright (c) 2026 The Corney Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/init.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>

#include <corney/input_split_safety.h>
#include <zmk/pointing/input_split.h>
#include <zmk/split/transport/central.h>
#include <zmk/split/transport/types.h>

LOG_MODULE_REGISTER(corney_input_split_test, LOG_LEVEL_INF);

#define TEST_RIGHT_INPUT_REG 0U
#define TEST_LEFT_INPUT_REG 1U
#define TEST_RIGHT_INPUT_NODE DT_NODELABEL(corney_input_split_test_right)
#define TEST_LEFT_INPUT_NODE DT_NODELABEL(corney_input_split_test_left)

static void log_right_proxy_event(struct input_event *event) {
  LOG_INF("reg=0 event type=%u code=%u value=%d sync=%u", event->type,
          event->code, event->value, event->sync);
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(TEST_RIGHT_INPUT_NODE),
                      log_right_proxy_event);

static void log_left_proxy_event(struct input_event *event) {
  LOG_INF("reg=1 event type=%u code=%u value=%d sync=%u", event->type,
          event->code, event->value, event->sync);
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(TEST_LEFT_INPUT_NODE),
                      log_left_proxy_event);

static int report_split_event(uint8_t reg, uint8_t type, uint16_t code,
                              int32_t value, bool sync) {
  const struct zmk_split_transport_peripheral_event event = {
      .type = ZMK_SPLIT_TRANSPORT_PERIPHERAL_EVENT_TYPE_INPUT_EVENT,
      .data = {.input_event = {.reg = reg,
                               .type = type,
                               .code = code,
                               .value = value,
                               .sync = sync}},
  };

  return zmk_split_transport_central_peripheral_event_handler(NULL, 0U, event);
}

static void reconnect_work_handler(struct k_work *work) {
  ARG_UNUSED(work);

  int err = report_split_event(TEST_LEFT_INPUT_REG, INPUT_EV_REL, INPUT_REL_X,
                               -20, true);
  if (err != 0) {
    LOG_ERR("reconnect event injection failed: %d", err);
  }
}

K_WORK_DELAYABLE_DEFINE(reconnect_work, reconnect_work_handler);

static void disconnect_work_handler(struct k_work *work) {
  ARG_UNUSED(work);

  int err = corney_input_split_release_buttons();
  if (err != 0) {
    LOG_ERR("disconnect release failed: %d", err);
    return;
  }

  k_work_reschedule(&reconnect_work, K_MSEC(10));
}

K_WORK_DELAYABLE_DEFINE(disconnect_work, disconnect_work_handler);

static void inject_work_handler(struct k_work *work) {
  ARG_UNUSED(work);
  int err;

  err = report_split_event(TEST_RIGHT_INPUT_REG, INPUT_EV_REL, INPUT_REL_X, 120,
                           false);
  if (err == 0) {
    err = report_split_event(TEST_RIGHT_INPUT_REG, INPUT_EV_REL, INPUT_REL_Y,
                             -45, true);
  }
  if (err == 0) {
    err = report_split_event(TEST_RIGHT_INPUT_REG, INPUT_EV_REL,
                             INPUT_REL_WHEEL, 2, true);
  }
  if (err == 0) {
    err = report_split_event(TEST_RIGHT_INPUT_REG, INPUT_EV_KEY, INPUT_BTN_0, 1,
                             true);
  }
  if (err == 0) {
    err = report_split_event(TEST_LEFT_INPUT_REG, INPUT_EV_REL, INPUT_REL_X, 75,
                             true);
  }
  if (err == 0) {
    err = report_split_event(TEST_LEFT_INPUT_REG, INPUT_EV_KEY, INPUT_BTN_1, 1,
                             true);
  }
  if (err == 0) {
    k_work_reschedule(&disconnect_work, K_MSEC(10));
  }

  if (err != 0) {
    LOG_ERR("event injection failed: %d", err);
  }
}

K_WORK_DELAYABLE_DEFINE(inject_work, inject_work_handler);

static int run_input_split_test(void) {
  k_work_schedule(&inject_work, K_MSEC(10));

  return 0;
}

SYS_INIT(run_input_split_test, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
