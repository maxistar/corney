/*
 * Copyright (c) 2026 Maxim Korenyugin
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>

#include <corney/pinnacle_packet.h>

#define PINNACLE_STATUS1_SW_DR (1U << 2)
#define PINNACLE_PACKET_X_SIGN (1U << 4)
#define PINNACLE_PACKET_Y_SIGN (1U << 5)

static int16_t decode_axis(uint8_t magnitude, bool negative) {
  return negative ? (int16_t)magnitude - 256 : magnitude;
}

bool corney_pinnacle_status_has_data(uint8_t status) {
  return status != UINT8_MAX && (status & PINNACLE_STATUS1_SW_DR) != 0U;
}

int corney_pinnacle_decode_relative(
    const uint8_t *packet, size_t packet_size,
    struct corney_pinnacle_relative_sample *sample) {
  if (packet == NULL || sample == NULL ||
      packet_size != CORNEY_PINNACLE_RELATIVE_PACKET_SIZE) {
    return -EINVAL;
  }

  sample->buttons = packet[0] & CORNEY_PINNACLE_BUTTON_MASK;
  sample->x =
      decode_axis(packet[1], (packet[0] & PINNACLE_PACKET_X_SIGN) != 0U);
  sample->y =
      decode_axis(packet[2], (packet[0] & PINNACLE_PACKET_Y_SIGN) != 0U);
  sample->wheel = (int8_t)packet[3];

  return 0;
}

int16_t corney_pinnacle_apply_axis_inversion(int16_t value, bool invert) {
  return invert ? -value : value;
}

uint8_t corney_pinnacle_button_changes(uint8_t previous, uint8_t current) {
  return (previous ^ current) & CORNEY_PINNACLE_BUTTON_MASK;
}
