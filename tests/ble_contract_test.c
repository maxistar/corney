#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <corney/ble_contract.h>
#include <corney/ble_frame_queue.h>
#include <corney/ble_subscription_owner.h>
#include <corney/ble_transport_policy.h>

static void assert_bytes(const uint8_t *actual, const uint8_t *expected,
                         size_t len) {
  assert(memcmp(actual, expected, len) == 0);
}

static void test_capabilities(void) {
  uint8_t actual[CORNEY_BLE_CAPABILITIES_SIZE];
  const uint8_t expected[] = {0x01, 0x00, 0x77, 0x00, 0x14, 0x01, 0x00, 0x00};
  const uint8_t none[] = {0x01, 0x00, 0x00, 0x00, 0x14, 0x01, 0x00, 0x00};
  const uint8_t independent[] = {0x01, 0x00, 0x31, 0x00,
                                 0x14, 0x01, 0x00, 0x00};

  corney_ble_encode_capabilities(UINT16_MAX, actual);
  assert_bytes(actual, expected, sizeof(expected));
  assert((actual[2] & CORNEY_BLE_CAP_RESERVED_3) == 0U);
  corney_ble_encode_capabilities(0, actual);
  assert_bytes(actual, none, sizeof(none));
  corney_ble_encode_capabilities(CORNEY_BLE_CAP_KEY_EVENTS |
                                     CORNEY_BLE_CAP_DIAGNOSTICS |
                                     CORNEY_BLE_CAP_LEGACY_LAYER_REGISTER,
                                 actual);
  assert_bytes(actual, independent, sizeof(independent));
}

static void test_key_frames(void) {
  struct corney_ble_frame frame;
  const uint8_t down[] = {0x01, 0x01, 0x00, 0x03, 0x2a, 0x00,
                          0x00, 0x00, 0x01, 0x01, 0x01};

  assert(corney_ble_encode_key(&frame, 0, 42, true, 1, 1) == 0);
  assert(frame.len == sizeof(down));
  assert_bytes(frame.data, down, sizeof(down));
  assert(corney_ble_validate_frame(&frame) == 0);
  assert(corney_ble_encode_key(&frame, 0, 0, true, 42, 0) == -ERANGE);
}

static void test_combo_frames(void) {
  struct corney_ble_frame frame;
  const uint8_t positions[] = {1, 2};
  const uint8_t expected[] = {0x01, 0x02, 0x00, 0x07, 0x2c, 0x00, 0x00, 0x00,
                              0x01, 0x00, 0x01, 0x01, 0x02, 0x01, 0x02};

  assert(corney_ble_encode_combo(&frame, 0, 44, 1, true, 1, positions,
                                 sizeof(positions)) == 0);
  assert(frame.len == sizeof(expected));
  assert_bytes(frame.data, expected, sizeof(expected));
  assert(corney_ble_encode_combo(&frame, 0, 0, 0, true, 0, positions, 2) ==
         -EINVAL);
  assert(corney_ble_encode_combo(&frame, 0, 0, 1, true, 0, positions, 5) ==
         -EINVAL);
  assert(corney_ble_encode_combo(&frame, 0, 0, 1, false, 0, NULL, 0) == 0);
}

static void test_other_frames(void) {
  struct corney_ble_frame frame;
  const uint8_t layer[] = {0x01, 0x03, 0x03, 0x04, 0x29, 0x00,
                           0x00, 0x00, 0x01, 0x01, 0x03, 0xff};
  const uint8_t diagnostic[] = {0x01, 0x05, 0x04, 0x0c, 0x31, 0x00, 0x00,
                                0x00, 0x01, 0x00, 0x01, 0x00, 0x03, 0x00,
                                0x00, 0x00, 0x30, 0x00, 0x00, 0x00};

  assert(corney_ble_encode_layer(
             &frame, CORNEY_BLE_FLAG_STREAM_START | CORNEY_BLE_FLAG_SNAPSHOT,
             41, 1, 1, CORNEY_BLE_LAYER_CAUSE_RESTORE, 0xff) == 0);
  assert_bytes(frame.data, layer, sizeof(layer));

  assert(corney_ble_encode_queue_overflow(&frame, 0, 49, 3, 48) == 0);
  assert_bytes(frame.data, diagnostic, sizeof(diagnostic));
  assert(frame.len == CORNEY_BLE_MAX_FRAME_SIZE);
}

static void test_validation(void) {
  struct corney_ble_frame frame = {.len = 8,
                                   .data = {1, 0xff, 0, 0, 0, 0, 0, 0}};
  uint8_t payload[CORNEY_BLE_MAX_PAYLOAD_SIZE + 1] = {0};

  assert(corney_ble_validate_frame(&frame) == 0);
  assert(corney_ble_encode_frame(&frame, CORNEY_BLE_EVENT_RESERVED_04, 0, 1,
                                 NULL, 0) == 0);
  assert(corney_ble_validate_frame(&frame) == 0);
  frame.data[0] = 2;
  assert(corney_ble_validate_frame(&frame) == -EINVAL);
  assert(corney_ble_encode_frame(&frame, 1, 0, 0, payload, sizeof(payload)) ==
         -EINVAL);
}

static void test_transport_policy(void) {
  struct corney_ble_frame frame = {.len = CORNEY_BLE_FRAME_HEADER_SIZE,
                                   .data = {CORNEY_BLE_PROTOCOL_MAJOR}};

  assert(CORNEY_BLE_STREAM_INITIAL_SEQUENCE == 0U);
  assert(corney_ble_transport_action_for_result(0, 0) ==
         CORNEY_BLE_TRANSPORT_ADVANCE);
  assert(corney_ble_transport_action_for_result(-ENOMEM, 0) ==
         CORNEY_BLE_TRANSPORT_RETRY);
  assert(corney_ble_transport_action_for_result(-EAGAIN, 2) ==
         CORNEY_BLE_TRANSPORT_RETRY);
  assert(corney_ble_transport_action_for_result(-ENOMEM, 3) ==
         CORNEY_BLE_TRANSPORT_DROP);
  assert(corney_ble_transport_action_for_result(-ENOTCONN, 0) ==
         CORNEY_BLE_TRANSPORT_DROP);
  assert(corney_ble_sequence_is_contiguous(41, 42));
  assert(corney_ble_sequence_is_contiguous(UINT32_MAX, 0));
  assert(!corney_ble_sequence_is_contiguous(41, 43));
  corney_ble_apply_stream_start(&frame, true);
  assert((frame.data[2] & CORNEY_BLE_FLAG_STREAM_START) != 0U);
  assert(corney_ble_stream_start_pending_after_result(true, -ENOTCONN));
  assert(!corney_ble_stream_start_pending_after_result(true, 0));
  assert(!corney_ble_stream_start_pending_after_result(false, -ENOTCONN));
}

static void test_bounded_queue(void) {
  struct corney_ble_frame storage[2];
  struct corney_ble_frame_queue queue;
  struct corney_ble_frame first;
  struct corney_ble_frame second;
  struct corney_ble_frame third;
  struct corney_ble_frame actual;

  corney_ble_frame_queue_init(&queue, storage, 2);
  assert(corney_ble_encode_key(&first, 0, 1, true, 1, 0) == 0);
  assert(corney_ble_encode_key(&second, 0, 2, false, 1, 0) == 0);
  assert(corney_ble_encode_key(&third, 0, 3, true, 2, 0) == 0);
  assert(corney_ble_frame_queue_push(&queue, &first) == 0);
  assert(corney_ble_frame_queue_push(&queue, &second) == 0);
  assert(corney_ble_frame_queue_push(&queue, &first) == -ENOSPC);
  assert(corney_ble_frame_queue_count(&queue) == 2);
  assert(corney_ble_frame_queue_pop(&queue, &actual) == 0);
  assert(actual.data[4] == 1);
  assert(corney_ble_frame_queue_push(&queue, &third) == 0);
  assert(corney_ble_frame_queue_pop(&queue, &actual) == 0);
  assert(actual.data[4] == 2);
  assert(corney_ble_frame_queue_pop(&queue, &actual) == 0);
  assert(actual.data[4] == 3);
  assert(corney_ble_frame_queue_pop(&queue, &actual) == -ENOENT);
  assert(corney_ble_frame_queue_push(&queue, &first) == 0);
  corney_ble_frame_queue_purge(&queue);
  assert(corney_ble_frame_queue_count(&queue) == 0);
}

static void test_subscription_owner(void) {
  struct corney_ble_subscription_owner owner = {0};
  int first = 1;
  int second = 2;

  assert(corney_ble_subscription_claim(&owner, &first) == 0);
  assert(corney_ble_subscription_claim(&owner, &first) == 0);
  assert(corney_ble_subscription_claim(&owner, &second) == -EBUSY);
  assert(corney_ble_subscription_is_owner(&owner, &first));
  assert(!corney_ble_subscription_release(&owner, &second));
  assert(corney_ble_subscription_release(&owner, &first));
  assert(corney_ble_subscription_claim(&owner, &second) == 0);
}

int main(void) {
  test_capabilities();
  test_key_frames();
  test_combo_frames();
  test_other_frames();
  test_validation();
  test_transport_policy();
  test_bounded_queue();
  test_subscription_owner();
  return 0;
}
