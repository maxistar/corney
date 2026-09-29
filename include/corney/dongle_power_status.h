#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CORNEY_DONGLE_HALF_COUNT 2U
#define CORNEY_DONGLE_POWER_TEXT_SIZE 16U

#define CORNEY_DONGLE_DISPLAY_WIDTH_PX 128U
#define CORNEY_DONGLE_DISPLAY_HEIGHT_PX 64U
#define CORNEY_DONGLE_STOCK_TOP_HEIGHT_PX 18U
#define CORNEY_DONGLE_STOCK_BOTTOM_HEIGHT_PX 15U
#define CORNEY_DONGLE_REMOTE_ROW_Y_PX 20U
#define CORNEY_DONGLE_REMOTE_ROW_HEIGHT_PX 15U
#define CORNEY_DONGLE_REMOTE_ROW_WIDTH_PX 65U
#define CORNEY_DONGLE_LAYER_ICON_WIDTH_PX 14U
#define CORNEY_DONGLE_LAYER_NAME_X_PX 16U
#define CORNEY_DONGLE_LAYER_NAME_WIDTH_PX 112U
#define CORNEY_DONGLE_LAYER_SCROLL_SPEED_PX_PER_SEC 20U

struct corney_dongle_half_power {
  uint8_t level;
  bool available;
};

struct corney_dongle_power_status {
  struct corney_dongle_half_power halves[CORNEY_DONGLE_HALF_COUNT];
};

void corney_dongle_power_status_init(struct corney_dongle_power_status *status);

int corney_dongle_power_status_set_half(
    struct corney_dongle_power_status *status, uint8_t source, uint8_t level);

int corney_dongle_power_status_format(
    const struct corney_dongle_power_status *status, char *buffer,
    size_t buffer_size);
