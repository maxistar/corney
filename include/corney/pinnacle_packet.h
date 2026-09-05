/*
 * Copyright (c) 2026 Maxim Korenyugin
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CORNEY_PINNACLE_RELATIVE_PACKET_SIZE 4U

#define CORNEY_PINNACLE_BUTTON_PRIMARY (1U << 0)
#define CORNEY_PINNACLE_BUTTON_SECONDARY (1U << 1)
#define CORNEY_PINNACLE_BUTTON_AUXILIARY (1U << 2)
#define CORNEY_PINNACLE_BUTTON_MASK                                            \
  (CORNEY_PINNACLE_BUTTON_PRIMARY | CORNEY_PINNACLE_BUTTON_SECONDARY |         \
   CORNEY_PINNACLE_BUTTON_AUXILIARY)

struct corney_pinnacle_relative_sample {
  int16_t x;
  int16_t y;
  int8_t wheel;
  uint8_t buttons;
};

bool corney_pinnacle_status_has_data(uint8_t status);
int corney_pinnacle_decode_relative(
    const uint8_t *packet, size_t packet_size,
    struct corney_pinnacle_relative_sample *sample);
int16_t corney_pinnacle_apply_axis_inversion(int16_t value, bool invert);
uint8_t corney_pinnacle_button_changes(uint8_t previous, uint8_t current);
