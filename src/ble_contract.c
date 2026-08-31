#include <errno.h>
#include <string.h>

#include <corney/ble_contract.h>

_Static_assert(CORNEY_BLE_FRAME_HEADER_SIZE + CORNEY_BLE_MAX_PAYLOAD_SIZE ==
                   CORNEY_BLE_MAX_FRAME_SIZE,
               "maximum BLE frame must fit the default ATT payload");
_Static_assert(CORNEY_BLE_FRAME_HEADER_SIZE + 3U <= CORNEY_BLE_MAX_FRAME_SIZE,
               "key frame exceeds the v1 frame limit");
_Static_assert(CORNEY_BLE_FRAME_HEADER_SIZE + 5U +
                       CORNEY_BLE_MAX_COMBO_POSITIONS <=
                   CORNEY_BLE_MAX_FRAME_SIZE,
               "combo frame exceeds the v1 frame limit");
_Static_assert(CORNEY_BLE_FRAME_HEADER_SIZE + 12U == CORNEY_BLE_MAX_FRAME_SIZE,
               "diagnostic frame must exactly fit the v1 frame limit");

static void put_le16(uint8_t *dst, uint16_t value) {
  dst[0] = (uint8_t)value;
  dst[1] = (uint8_t)(value >> 8);
}

static void put_le32(uint8_t *dst, uint32_t value) {
  dst[0] = (uint8_t)value;
  dst[1] = (uint8_t)(value >> 8);
  dst[2] = (uint8_t)(value >> 16);
  dst[3] = (uint8_t)(value >> 24);
}

void corney_ble_encode_capabilities(uint16_t flags,
                                    uint8_t out[CORNEY_BLE_CAPABILITIES_SIZE]) {
  memset(out, 0, CORNEY_BLE_CAPABILITIES_SIZE);
  out[0] = CORNEY_BLE_PROTOCOL_MAJOR;
  out[1] = CORNEY_BLE_PROTOCOL_MINOR;
  put_le16(&out[2], flags & CORNEY_BLE_CAPABILITY_V1_MASK);
  out[4] = CORNEY_BLE_MAX_FRAME_SIZE;
  out[5] = CORNEY_BLE_POSITION_SCHEMA;
}

int corney_ble_encode_frame(struct corney_ble_frame *frame, uint8_t type,
                            uint8_t flags, uint32_t sequence,
                            const uint8_t *payload, size_t payload_len) {
  if (frame == NULL || payload_len > CORNEY_BLE_MAX_PAYLOAD_SIZE ||
      (payload_len > 0U && payload == NULL)) {
    return -EINVAL;
  }

  frame->len = (uint8_t)(CORNEY_BLE_FRAME_HEADER_SIZE + payload_len);
  frame->data[0] = CORNEY_BLE_PROTOCOL_MAJOR;
  frame->data[1] = type;
  frame->data[2] = flags;
  frame->data[3] = (uint8_t)payload_len;
  put_le32(&frame->data[4], sequence);
  if (payload_len > 0U) {
    memcpy(&frame->data[CORNEY_BLE_FRAME_HEADER_SIZE], payload, payload_len);
  }
  return 0;
}

int corney_ble_encode_key(struct corney_ble_frame *frame, uint8_t flags,
                          uint32_t sequence, bool pressed, uint8_t position,
                          uint8_t layer) {
  uint8_t payload[3] = {pressed ? CORNEY_BLE_ACTION_DOWN : CORNEY_BLE_ACTION_UP,
                        position, layer};

  if (position > CORNEY_BLE_MAX_POSITION) {
    return -ERANGE;
  }
  return corney_ble_encode_frame(frame, CORNEY_BLE_EVENT_KEY, flags, sequence,
                                 payload, sizeof(payload));
}

int corney_ble_encode_combo(struct corney_ble_frame *frame, uint8_t flags,
                            uint32_t sequence, uint16_t combo_id, bool pressed,
                            uint8_t layer, const uint8_t *positions,
                            size_t position_count) {
  uint8_t payload[5U + CORNEY_BLE_MAX_COMBO_POSITIONS] = {0};

  if (combo_id == 0U || position_count > CORNEY_BLE_MAX_COMBO_POSITIONS ||
      (position_count > 0U && positions == NULL)) {
    return -EINVAL;
  }
  for (size_t i = 0; i < position_count; i++) {
    if (positions[i] > CORNEY_BLE_MAX_POSITION) {
      return -ERANGE;
    }
  }

  put_le16(payload, combo_id);
  payload[2] = pressed ? CORNEY_BLE_ACTION_DOWN : CORNEY_BLE_ACTION_UP;
  payload[3] = layer;
  payload[4] = (uint8_t)position_count;
  if (position_count > 0U) {
    memcpy(&payload[5], positions, position_count);
  }
  return corney_ble_encode_frame(frame, CORNEY_BLE_EVENT_COMBO, flags, sequence,
                                 payload, 5U + position_count);
}

int corney_ble_encode_layer(struct corney_ble_frame *frame, uint8_t flags,
                            uint32_t sequence, uint8_t layer,
                            uint8_t previous_layer, uint8_t cause,
                            uint8_t origin_position) {
  uint8_t payload[4] = {layer, previous_layer, cause, origin_position};
  return corney_ble_encode_frame(frame, CORNEY_BLE_EVENT_LAYER, flags, sequence,
                                 payload, sizeof(payload));
}

int corney_ble_encode_queue_overflow(struct corney_ble_frame *frame,
                                     uint8_t flags, uint32_t sequence,
                                     uint32_t count,
                                     uint32_t discarded_sequence) {
  uint8_t payload[12] = {0};

  put_le16(payload, CORNEY_BLE_DIAGNOSTIC_QUEUE_OVERFLOW);
  payload[2] = CORNEY_BLE_DIAGNOSTIC_WARNING;
  payload[3] = CORNEY_BLE_DIAGNOSTIC_SOURCE_CENTRAL_LEFT;
  put_le32(&payload[4], count);
  put_le32(&payload[8], discarded_sequence);
  return corney_ble_encode_frame(frame, CORNEY_BLE_EVENT_DIAGNOSTIC,
                                 flags | CORNEY_BLE_FLAG_GAP_REPORT, sequence,
                                 payload, sizeof(payload));
}

int corney_ble_validate_frame(const struct corney_ble_frame *frame) {
  if (frame == NULL || frame->len < CORNEY_BLE_FRAME_HEADER_SIZE ||
      frame->len > CORNEY_BLE_MAX_FRAME_SIZE ||
      frame->data[0] != CORNEY_BLE_PROTOCOL_MAJOR ||
      frame->data[3] > CORNEY_BLE_MAX_PAYLOAD_SIZE ||
      frame->len != CORNEY_BLE_FRAME_HEADER_SIZE + frame->data[3]) {
    return -EINVAL;
  }
  return 0;
}
