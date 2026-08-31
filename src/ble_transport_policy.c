#include <errno.h>

#include <corney/ble_transport_policy.h>

enum corney_ble_transport_action
corney_ble_transport_action_for_result(int result, uint8_t retries_used) {
  if (result == 0) {
    return CORNEY_BLE_TRANSPORT_ADVANCE;
  }
  if ((result == -ENOMEM || result == -EAGAIN) &&
      retries_used < CORNEY_BLE_TRANSPORT_MAX_RETRIES) {
    return CORNEY_BLE_TRANSPORT_RETRY;
  }
  return CORNEY_BLE_TRANSPORT_DROP;
}

bool corney_ble_sequence_is_contiguous(uint32_t previous, uint32_t current) {
  return current - previous == 1U;
}

void corney_ble_apply_stream_start(struct corney_ble_frame *frame,
                                   bool pending) {
  if (pending) {
    frame->data[2] |= CORNEY_BLE_FLAG_STREAM_START;
  }
}

bool corney_ble_stream_start_pending_after_result(bool pending,
                                                  int notify_result) {
  return pending && notify_result != 0;
}
