#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <corney/ble_contract.h>

#define CORNEY_BLE_TRANSPORT_RETRY_MS 10U
#define CORNEY_BLE_TRANSPORT_MAX_RETRIES 3U

enum corney_ble_transport_action {
  CORNEY_BLE_TRANSPORT_ADVANCE,
  CORNEY_BLE_TRANSPORT_RETRY,
  CORNEY_BLE_TRANSPORT_DROP,
};

enum corney_ble_transport_action
corney_ble_transport_action_for_result(int result, uint8_t retries_used);
bool corney_ble_sequence_is_contiguous(uint32_t previous, uint32_t current);
void corney_ble_apply_stream_start(struct corney_ble_frame *frame,
                                   bool pending);
bool corney_ble_stream_start_pending_after_result(bool pending,
                                                  int notify_result);
