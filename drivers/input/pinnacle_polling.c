/*
 * Copyright (c) 2024 The ZMK Contributors
 * Copyright (c) 2026 Maxim Korenyugin
 *
 * SPDX-License-Identifier: MIT
 *
 * Register access and initialization are adapted from
 * https://github.com/petejohanson/cirque-input-module.
 */

#define DT_DRV_COMPAT corney_cirque_pinnacle_polling

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/util.h>

#include <corney/pinnacle_packet.h>

LOG_MODULE_REGISTER(corney_pinnacle, CONFIG_INPUT_LOG_LEVEL);

#define PINNACLE_READ_MASK 0xA0U
#define PINNACLE_WRITE_MASK 0x80U

#define PINNACLE_REG_FIRMWARE_ID 0x00U
#define PINNACLE_REG_STATUS1 0x02U
#define PINNACLE_REG_SYS_CONFIG1 0x03U
#define PINNACLE_REG_FEED_CONFIG1 0x04U
#define PINNACLE_REG_FEED_CONFIG2 0x05U
#define PINNACLE_REG_CAL_CONFIG1 0x07U
#define PINNACLE_REG_Z_IDLE 0x0AU
#define PINNACLE_REG_PACKET_BYTE0 0x12U
#define PINNACLE_REG_ERA_VALUE 0x1BU
#define PINNACLE_REG_ERA_ADDR_HIGH 0x1CU
#define PINNACLE_REG_ERA_ADDR_LOW 0x1DU
#define PINNACLE_REG_ERA_CONTROL 0x1EU

#define PINNACLE_FIRMWARE_ID 0x07U
#define PINNACLE_SYS_CONFIG_RESET BIT(0)
#define PINNACLE_SYS_CONFIG_LOW_POWER BIT(2)
#define PINNACLE_FEED_ENABLE BIT(0)
#define PINNACLE_FEED_INTELLIMOUSE BIT(0)
#define PINNACLE_FEED_DISABLE_TAPS BIT(1)
#define PINNACLE_FEED_BUTTON_SCROLL BIT(6)
#define PINNACLE_CALIBRATE BIT(0)
#define PINNACLE_ERA_READ BIT(0)
#define PINNACLE_ERA_WRITE BIT(1)
#define PINNACLE_ERA_ADC_CONFIG 0x0187U
#define PINNACLE_ERA_ADC_MASK 0xC0U

#define PINNACLE_RESET_DELAY_MS 30U
#define PINNACLE_COMMAND_POLL_US 1000U
#define PINNACLE_COMMAND_ATTEMPTS 50U
#define PINNACLE_CALIBRATION_POLL_MS 50U
#define PINNACLE_CALIBRATION_ATTEMPTS 4U
#define PINNACLE_ERROR_LOG_INTERVAL 128U

struct pinnacle_polling_config {
  struct i2c_dt_spec bus;
  uint16_t poll_interval_ms;
  uint8_t sensitivity;
  bool primary_tap_enabled;
  bool invert_x;
  bool sleep_mode_enabled;
};

struct pinnacle_polling_data {
  struct k_work_delayable poll_work;
  const struct device *dev;
  uint32_t consecutive_errors;
  uint8_t buttons;
  bool suspended;
};

static int pinnacle_read(const struct device *dev, uint8_t reg, uint8_t *buffer,
                         size_t size) {
  const struct pinnacle_polling_config *config = dev->config;

  return i2c_burst_read_dt(&config->bus, PINNACLE_READ_MASK | reg, buffer,
                           size);
}

static int pinnacle_write(const struct device *dev, uint8_t reg,
                          uint8_t value) {
  const struct pinnacle_polling_config *config = dev->config;

  return i2c_reg_write_byte_dt(&config->bus, PINNACLE_WRITE_MASK | reg, value);
}

static int pinnacle_wait_for_era(const struct device *dev) {
  uint8_t control;

  for (uint32_t attempt = 0U; attempt < PINNACLE_COMMAND_ATTEMPTS; attempt++) {
    int err =
        pinnacle_read(dev, PINNACLE_REG_ERA_CONTROL, &control, sizeof(control));

    if (err != 0) {
      return err;
    }
    if (control == 0U) {
      return 0;
    }
    k_usleep(PINNACLE_COMMAND_POLL_US);
  }

  return -ETIMEDOUT;
}

static int pinnacle_era_read(const struct device *dev, uint16_t address,
                             uint8_t *value) {
  int err;

  err =
      pinnacle_write(dev, PINNACLE_REG_ERA_ADDR_HIGH, (uint8_t)(address >> 8));
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_ERA_ADDR_LOW, (uint8_t)address);
  }
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_ERA_CONTROL, PINNACLE_ERA_READ);
  }
  if (err == 0) {
    err = pinnacle_wait_for_era(dev);
  }
  if (err == 0) {
    err = pinnacle_read(dev, PINNACLE_REG_ERA_VALUE, value, sizeof(*value));
  }
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_STATUS1, 0U);
  }

  return err;
}

static int pinnacle_era_write(const struct device *dev, uint16_t address,
                              uint8_t value) {
  int err;

  err = pinnacle_write(dev, PINNACLE_REG_ERA_VALUE, value);
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_ERA_ADDR_HIGH,
                         (uint8_t)(address >> 8));
  }
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_ERA_ADDR_LOW, (uint8_t)address);
  }
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_ERA_CONTROL, PINNACLE_ERA_WRITE);
  }
  if (err == 0) {
    err = pinnacle_wait_for_era(dev);
  }
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_STATUS1, 0U);
  }

  return err;
}

static int pinnacle_set_low_power(const struct device *dev, bool enabled) {
  uint8_t config;
  int err =
      pinnacle_read(dev, PINNACLE_REG_SYS_CONFIG1, &config, sizeof(config));

  if (err != 0) {
    return err;
  }

  if (enabled) {
    config |= PINNACLE_SYS_CONFIG_LOW_POWER;
  } else {
    config &= ~PINNACLE_SYS_CONFIG_LOW_POWER;
  }

  return pinnacle_write(dev, PINNACLE_REG_SYS_CONFIG1, config);
}

static int pinnacle_configure_sensitivity(const struct device *dev) {
  const struct pinnacle_polling_config *config = dev->config;
  uint8_t adc_config;
  int err = pinnacle_era_read(dev, PINNACLE_ERA_ADC_CONFIG, &adc_config);

  if (err != 0) {
    return err;
  }

  adc_config =
      (adc_config & ~PINNACLE_ERA_ADC_MASK) | (config->sensitivity << 6);
  return pinnacle_era_write(dev, PINNACLE_ERA_ADC_CONFIG, adc_config);
}

static int pinnacle_calibrate(const struct device *dev) {
  uint8_t calibration;
  int err = pinnacle_read(dev, PINNACLE_REG_CAL_CONFIG1, &calibration,
                          sizeof(calibration));

  if (err != 0) {
    return err;
  }

  err = pinnacle_write(dev, PINNACLE_REG_CAL_CONFIG1,
                       calibration | PINNACLE_CALIBRATE);
  if (err != 0) {
    return err;
  }

  for (uint32_t attempt = 0U; attempt < PINNACLE_CALIBRATION_ATTEMPTS;
       attempt++) {
    k_msleep(PINNACLE_CALIBRATION_POLL_MS);
    err = pinnacle_read(dev, PINNACLE_REG_CAL_CONFIG1, &calibration,
                        sizeof(calibration));
    if (err != 0) {
      return err;
    }
    if ((calibration & PINNACLE_CALIBRATE) == 0U) {
      return 0;
    }
  }

  return -ETIMEDOUT;
}

static int pinnacle_configure(const struct device *dev) {
  const struct pinnacle_polling_config *config = dev->config;
  uint8_t firmware_id;
  int err;

  err = pinnacle_read(dev, PINNACLE_REG_FIRMWARE_ID, &firmware_id,
                      sizeof(firmware_id));
  if (err != 0) {
    return err;
  }
  if (firmware_id != PINNACLE_FIRMWARE_ID) {
    LOG_WRN("unexpected firmware ID 0x%02x", firmware_id);
  }

  err = pinnacle_write(dev, PINNACLE_REG_STATUS1, 0U);
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_SYS_CONFIG1,
                         PINNACLE_SYS_CONFIG_RESET);
  }
  if (err != 0) {
    return err;
  }

  k_msleep(PINNACLE_RESET_DELAY_MS);

  err = pinnacle_write(dev, PINNACLE_REG_Z_IDLE, 5U);
  if (err == 0) {
    err = pinnacle_configure_sensitivity(dev);
  }
  if (err == 0) {
    err = pinnacle_calibrate(dev);
  }
  if (err == 0) {
    uint8_t feed_config2 =
        PINNACLE_FEED_INTELLIMOUSE | PINNACLE_FEED_BUTTON_SCROLL;

    if (!config->primary_tap_enabled) {
      feed_config2 |= PINNACLE_FEED_DISABLE_TAPS;
    }
    err = pinnacle_write(dev, PINNACLE_REG_FEED_CONFIG2, feed_config2);
  }
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_FEED_CONFIG1, PINNACLE_FEED_ENABLE);
  }
  if (err == 0 && config->sleep_mode_enabled) {
    err = pinnacle_set_low_power(dev, true);
  }
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_STATUS1, 0U);
  }

  return err;
}

static void
pinnacle_report_buttons(const struct device *dev,
                        const struct corney_pinnacle_relative_sample *sample) {
  const struct pinnacle_polling_config *config = dev->config;
  struct pinnacle_polling_data *data = dev->data;
  uint8_t changes;

  if (!config->primary_tap_enabled) {
    return;
  }

  changes = corney_pinnacle_button_changes(data->buttons, sample->buttons);
  for (uint8_t index = 0U; index < 3U; index++) {
    uint8_t mask = BIT(index);

    if ((changes & mask) != 0U) {
      input_report_key(dev, INPUT_BTN_0 + index, (sample->buttons & mask) != 0U,
                       false, K_NO_WAIT);
    }
  }
  data->buttons = sample->buttons;
}

static int pinnacle_poll_once(const struct device *dev) {
  const struct pinnacle_polling_config *config = dev->config;
  uint8_t status;
  uint8_t packet[CORNEY_PINNACLE_RELATIVE_PACKET_SIZE];
  struct corney_pinnacle_relative_sample sample;
  int err = pinnacle_read(dev, PINNACLE_REG_STATUS1, &status, sizeof(status));

  if (err != 0 || !corney_pinnacle_status_has_data(status)) {
    return err;
  }

  err = pinnacle_read(dev, PINNACLE_REG_PACKET_BYTE0, packet, sizeof(packet));
  if (err == 0) {
    err = pinnacle_write(dev, PINNACLE_REG_STATUS1, 0U);
  }
  if (err == 0) {
    err = corney_pinnacle_decode_relative(packet, sizeof(packet), &sample);
  }
  if (err != 0) {
    return err;
  }

  sample.x = corney_pinnacle_apply_axis_inversion(sample.x, config->invert_x);

  pinnacle_report_buttons(dev, &sample);
  if (sample.wheel != 0) {
    return input_report_rel(dev, INPUT_REL_WHEEL, sample.wheel, true,
                            K_NO_WAIT);
  }

  input_report_rel(dev, INPUT_REL_X, sample.x, false, K_NO_WAIT);
  return input_report_rel(dev, INPUT_REL_Y, sample.y, true, K_NO_WAIT);
}

static void pinnacle_poll_handler(struct k_work *work) {
  struct k_work_delayable *delayable = k_work_delayable_from_work(work);
  struct pinnacle_polling_data *data =
      CONTAINER_OF(delayable, struct pinnacle_polling_data, poll_work);
  const struct pinnacle_polling_config *config = data->dev->config;
  int err;

  if (data->suspended) {
    return;
  }

  err = pinnacle_poll_once(data->dev);
  if (err != 0) {
    data->consecutive_errors++;
    if (data->consecutive_errors == 1U ||
        (data->consecutive_errors % PINNACLE_ERROR_LOG_INTERVAL) == 0U) {
      LOG_WRN("I2C poll failed (%d), consecutive failures: %u", err,
              data->consecutive_errors);
    }
  } else {
    data->consecutive_errors = 0U;
  }

  if (!data->suspended) {
    k_work_reschedule(&data->poll_work, K_MSEC(config->poll_interval_ms));
  }
}

static int pinnacle_init(const struct device *dev) {
  const struct pinnacle_polling_config *config = dev->config;
  struct pinnacle_polling_data *data = dev->data;
  int err;

  if (!i2c_is_ready_dt(&config->bus)) {
    LOG_ERR("I2C bus is not ready");
    return -ENODEV;
  }

  data->dev = dev;
  k_work_init_delayable(&data->poll_work, pinnacle_poll_handler);

  err = pinnacle_configure(dev);
  if (err != 0) {
    LOG_ERR("sensor initialization failed: %d", err);
    return err;
  }

  k_work_schedule(&data->poll_work, K_MSEC(config->poll_interval_ms));
  return 0;
}

#if IS_ENABLED(CONFIG_PM_DEVICE)
static int pinnacle_pm_action(const struct device *dev,
                              enum pm_device_action action) {
  const struct pinnacle_polling_config *config = dev->config;
  struct pinnacle_polling_data *data = dev->data;
  int err;

  switch (action) {
  case PM_DEVICE_ACTION_SUSPEND:
    data->suspended = true;
    k_work_cancel_delayable(&data->poll_work);
    return pinnacle_set_low_power(dev, true);
  case PM_DEVICE_ACTION_RESUME:
    err = pinnacle_configure(dev);
    if (err != 0) {
      return err;
    }
    data->suspended = false;
    data->consecutive_errors = 0U;
    k_work_reschedule(&data->poll_work, K_MSEC(config->poll_interval_ms));
    return 0;
  default:
    return -ENOTSUP;
  }
}
#endif

#define PINNACLE_POLLING_DEFINE(inst)                                          \
  BUILD_ASSERT(DT_INST_PROP(inst, poll_interval_ms) > 0,                       \
               "poll-interval-ms must be greater than zero");                  \
  static struct pinnacle_polling_data pinnacle_polling_data_##inst;            \
  static const struct pinnacle_polling_config pinnacle_polling_config_##inst = \
      {                                                                        \
          .bus = I2C_DT_SPEC_INST_GET(inst),                                   \
          .poll_interval_ms = DT_INST_PROP(inst, poll_interval_ms),            \
          .sensitivity = DT_INST_ENUM_IDX(inst, sensitivity),                  \
          .primary_tap_enabled = DT_INST_PROP(inst, primary_tap_enable),       \
          .invert_x = DT_INST_PROP(inst, invert_x),                            \
          .sleep_mode_enabled = DT_INST_PROP(inst, sleep_mode_enable),         \
  };                                                                           \
  PM_DEVICE_DT_INST_DEFINE(inst, pinnacle_pm_action);                          \
  DEVICE_DT_INST_DEFINE(inst, pinnacle_init, PM_DEVICE_DT_INST_GET(inst),      \
                        &pinnacle_polling_data_##inst,                         \
                        &pinnacle_polling_config_##inst, POST_KERNEL,          \
                        CONFIG_INPUT_CORNEY_PINNACLE_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(PINNACLE_POLLING_DEFINE)
