#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CORNEY_DONGLE_HALF_COUNT 2U
#define CORNEY_DONGLE_POWER_TEXT_SIZE 64U

struct corney_dongle_half_power {
  uint8_t level;
  bool available;
};

struct corney_dongle_power_status {
  uint8_t dongle_level;
  bool usb_powered;
  struct corney_dongle_half_power halves[CORNEY_DONGLE_HALF_COUNT];
};

void corney_dongle_power_status_init(struct corney_dongle_power_status *status,
                                     uint8_t dongle_level, bool usb_powered);

void corney_dongle_power_status_set_local(
    struct corney_dongle_power_status *status, uint8_t level);

void corney_dongle_power_status_set_usb(
    struct corney_dongle_power_status *status, bool powered);

int corney_dongle_power_status_set_half(
    struct corney_dongle_power_status *status, uint8_t source, uint8_t level);

int corney_dongle_power_status_format(
    const struct corney_dongle_power_status *status, char *buffer,
    size_t buffer_size);
