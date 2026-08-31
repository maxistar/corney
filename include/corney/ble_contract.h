#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CORNEY_BLE_PROTOCOL_MAJOR 1U
#define CORNEY_BLE_PROTOCOL_MINOR 0U
#define CORNEY_BLE_CAPABILITIES_SIZE 8U
#define CORNEY_BLE_FRAME_HEADER_SIZE 8U
#define CORNEY_BLE_MAX_PAYLOAD_SIZE 12U
#define CORNEY_BLE_MAX_FRAME_SIZE 20U
#define CORNEY_BLE_POSITION_SCHEMA 1U
#define CORNEY_BLE_STREAM_INITIAL_SEQUENCE 0U
#define CORNEY_BLE_MAX_POSITION 41U
#define CORNEY_BLE_MAX_COMBO_POSITIONS 4U

enum corney_ble_capability {
  CORNEY_BLE_CAP_KEY_EVENTS = 1U << 0,
  CORNEY_BLE_CAP_COMBO_EVENTS = 1U << 1,
  CORNEY_BLE_CAP_LAYER_EVENTS = 1U << 2,
  CORNEY_BLE_CAP_RESERVED_3 = 1U << 3,
  CORNEY_BLE_CAP_DIAGNOSTICS = 1U << 4,
  CORNEY_BLE_CAP_LEGACY_LAYER_REGISTER = 1U << 5,
  CORNEY_BLE_CAP_LEGACY_LAYER_WRITE = 1U << 6,
};

#define CORNEY_BLE_CAPABILITY_V1_MASK                                          \
  (CORNEY_BLE_CAP_KEY_EVENTS | CORNEY_BLE_CAP_COMBO_EVENTS |                   \
   CORNEY_BLE_CAP_LAYER_EVENTS | CORNEY_BLE_CAP_DIAGNOSTICS |                  \
   CORNEY_BLE_CAP_LEGACY_LAYER_REGISTER | CORNEY_BLE_CAP_LEGACY_LAYER_WRITE)

enum corney_ble_event_type {
  CORNEY_BLE_EVENT_KEY = 0x01,
  CORNEY_BLE_EVENT_COMBO = 0x02,
  CORNEY_BLE_EVENT_LAYER = 0x03,
  CORNEY_BLE_EVENT_RESERVED_04 = 0x04,
  CORNEY_BLE_EVENT_DIAGNOSTIC = 0x05,
};

enum corney_ble_frame_flag {
  CORNEY_BLE_FLAG_STREAM_START = 1U << 0,
  CORNEY_BLE_FLAG_SNAPSHOT = 1U << 1,
  CORNEY_BLE_FLAG_GAP_REPORT = 1U << 2,
};

enum corney_ble_action {
  CORNEY_BLE_ACTION_UP = 0,
  CORNEY_BLE_ACTION_DOWN = 1,
};

enum corney_ble_layer_cause {
  CORNEY_BLE_LAYER_CAUSE_UNKNOWN = 0,
  CORNEY_BLE_LAYER_CAUSE_PHYSICAL = 1,
  CORNEY_BLE_LAYER_CAUSE_REMOTE_WRITE = 2,
  CORNEY_BLE_LAYER_CAUSE_RESTORE = 3,
};

enum corney_ble_diagnostic_source {
  CORNEY_BLE_DIAGNOSTIC_SOURCE_CENTRAL_LEFT = 0,
};

enum corney_ble_diagnostic_code {
  CORNEY_BLE_DIAGNOSTIC_QUEUE_OVERFLOW = 1,
};

enum corney_ble_diagnostic_severity {
  CORNEY_BLE_DIAGNOSTIC_INFO = 0,
  CORNEY_BLE_DIAGNOSTIC_WARNING = 1,
  CORNEY_BLE_DIAGNOSTIC_ERROR = 2,
};

struct corney_ble_frame {
  uint8_t len;
  uint8_t data[CORNEY_BLE_MAX_FRAME_SIZE];
};

void corney_ble_encode_capabilities(uint16_t flags,
                                    uint8_t out[CORNEY_BLE_CAPABILITIES_SIZE]);
int corney_ble_encode_frame(struct corney_ble_frame *frame, uint8_t type,
                            uint8_t flags, uint32_t sequence,
                            const uint8_t *payload, size_t payload_len);
int corney_ble_encode_key(struct corney_ble_frame *frame, uint8_t flags,
                          uint32_t sequence, bool pressed, uint8_t position,
                          uint8_t layer);
int corney_ble_encode_combo(struct corney_ble_frame *frame, uint8_t flags,
                            uint32_t sequence, uint16_t combo_id, bool pressed,
                            uint8_t layer, const uint8_t *positions,
                            size_t position_count);
int corney_ble_encode_layer(struct corney_ble_frame *frame, uint8_t flags,
                            uint32_t sequence, uint8_t layer,
                            uint8_t previous_layer, uint8_t cause,
                            uint8_t origin_position);
int corney_ble_encode_queue_overflow(struct corney_ble_frame *frame,
                                     uint8_t flags, uint32_t sequence,
                                     uint32_t count,
                                     uint32_t discarded_sequence);
int corney_ble_validate_frame(const struct corney_ble_frame *frame);
