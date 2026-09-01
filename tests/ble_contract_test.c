#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <corney/ble_contract.h>
#include <corney/ble_frame_history.h>
#include <corney/ble_subscriber_registry.h>
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

static void test_bounded_history(void) {
  struct corney_ble_history_entry storage[2];
  struct corney_ble_frame_history history;
  struct corney_ble_frame first;
  struct corney_ble_frame second;
  struct corney_ble_frame third;
  struct corney_ble_frame actual;
  uint64_t entry_id;
  bool overwrote;

  corney_ble_frame_history_init(&history, storage, 2);
  assert(corney_ble_encode_key(&first, 0, 1, true, 1, 0) == 0);
  assert(corney_ble_encode_key(&second, 0, 2, false, 1, 0) == 0);
  assert(corney_ble_encode_key(&third, 0, 3, true, 2, 0) == 0);
  assert(corney_ble_frame_history_push(&history, &first, &entry_id,
                                       &overwrote) == 0);
  assert(entry_id == 0U && !overwrote);
  assert(corney_ble_frame_history_push(&history, &second, &entry_id,
                                       &overwrote) == 0);
  assert(entry_id == 1U && !overwrote);
  assert(corney_ble_frame_history_count(&history) == 2U);
  assert(corney_ble_frame_history_get(&history, 0U, &actual) == 0);
  assert(actual.data[4] == 1U);

  assert(corney_ble_frame_history_push(&history, &third, &entry_id,
                                       &overwrote) == 0);
  assert(entry_id == 2U && overwrote);
  assert(corney_ble_frame_history_oldest_id(&history) == 1U);
  assert(corney_ble_frame_history_next_id(&history) == 3U);
  assert(corney_ble_frame_history_get(&history, 0U, &actual) == -ERANGE);
  assert(corney_ble_frame_history_get(&history, 1U, &actual) == 0);
  assert(actual.data[4] == 2U);
  assert(corney_ble_frame_history_get(&history, 2U, &actual) == 0);
  assert(actual.data[4] == 3U);

  corney_ble_frame_history_discard_before(&history, 2U);
  assert(corney_ble_frame_history_count(&history) == 1U);
  assert(corney_ble_frame_history_oldest_id(&history) == 2U);
  corney_ble_frame_history_purge(&history);
  assert(corney_ble_frame_history_count(&history) == 0U);
  assert(corney_ble_frame_history_next_id(&history) == 3U);
}

static void test_subscriber_registry(void) {
  struct corney_ble_subscriber slots[2];
  struct corney_ble_subscriber_registry registry;
  int first = 1;
  int second = 2;
  int third = 3;
  size_t index;
  bool added;

  corney_ble_subscriber_registry_init(&registry, slots, 2U);
  assert(corney_ble_subscriber_add(&registry, &first, &index, &added) == 0);
  assert(index == 0U && added);
  slots[index].state = CORNEY_BLE_SUBSCRIBER_INITIALIZING;
  slots[index].snapshot_sequence = UINT32_MAX;
  slots[index].next_entry_id = 0U;

  assert(corney_ble_subscriber_add(&registry, &first, &index, &added) == 0);
  assert(index == 0U && !added);
  assert(slots[index].state == CORNEY_BLE_SUBSCRIBER_INITIALIZING);
  assert(corney_ble_subscriber_add(&registry, &second, &index, &added) == 0);
  assert(index == 1U && added);
  assert(corney_ble_subscriber_add(&registry, &third, &index, &added) ==
         -ENOSPC);
  assert(corney_ble_subscriber_count(&registry) == 2U);
  assert(corney_ble_subscriber_find(&registry, &first) == &slots[0]);
  assert(corney_ble_subscriber_find_const(&registry, &second) == &slots[1]);

  assert(corney_ble_subscriber_next_occupied(&registry) == 0U);
  assert(corney_ble_subscriber_next_occupied(&registry) == 1U);
  assert(corney_ble_subscriber_next_occupied(&registry) == 0U);
  assert(!corney_ble_subscriber_remove(&registry, &third));
  assert(corney_ble_subscriber_remove(&registry, &first));
  assert(corney_ble_subscriber_count(&registry) == 1U);
  assert(corney_ble_subscriber_add(&registry, &third, &index, &added) == 0);
  assert(index == 0U && added);
  assert(corney_ble_subscriber_next_occupied(&registry) == 1U);
  assert(corney_ble_subscriber_next_occupied(&registry) == 0U);
}

static void test_subscriber_scheduling_and_lag_isolation(void) {
  struct corney_ble_subscriber slots[3];
  struct corney_ble_subscriber_registry registry;
  int first = 1;
  int second = 2;
  int third = 3;
  size_t index;
  bool added;

  corney_ble_subscriber_registry_init(&registry, slots, 3U);
  assert(corney_ble_subscriber_add(&registry, &first, &index, &added) == 0);
  slots[index].state = CORNEY_BLE_SUBSCRIBER_ACTIVE;
  slots[index].next_entry_id = 8U;
  slots[index].retry_at_ms = 110;
  slots[index].retry_count = 2U;
  assert(corney_ble_subscriber_add(&registry, &second, &index, &added) == 0);
  slots[index].state = CORNEY_BLE_SUBSCRIBER_ACTIVE;
  slots[index].next_entry_id = 9U;
  assert(corney_ble_subscriber_add(&registry, &third, &index, &added) == 0);
  slots[index].state = CORNEY_BLE_SUBSCRIBER_INITIALIZING;

  /* The retrying first subscriber does not block the ready second one. */
  assert(corney_ble_subscriber_next_ready(&registry, 8U, 10U, 100) == 1U);
  /* Round-robin proceeds to the joining subscriber before revisiting first. */
  assert(corney_ble_subscriber_next_ready(&registry, 8U, 10U, 100) == 2U);
  assert(corney_ble_subscriber_next_delay_ms(&registry, 8U, 10U, 100) == 0);

  slots[1].next_entry_id = 10U;
  slots[2].state = CORNEY_BLE_SUBSCRIBER_ACTIVE;
  slots[2].next_entry_id = 10U;
  assert(corney_ble_subscriber_next_ready(&registry, 8U, 10U, 100) == SIZE_MAX);
  assert(corney_ble_subscriber_next_delay_ms(&registry, 8U, 10U, 100) == 10);

  /* Retention overrun advances only the lagging cursor and clears its retry. */
  slots[0].next_entry_id = 4U;
  assert(corney_ble_subscriber_next_ready(&registry, 7U, 10U, 100) == 0U);
  assert(slots[0].next_entry_id == 7U);
  assert(slots[0].retry_count == 0U);
  assert(slots[0].retry_at_ms == 0);
  assert(slots[1].next_entry_id == 10U);
  assert(slots[2].next_entry_id == 10U);
}

int main(void) {
  test_capabilities();
  test_key_frames();
  test_combo_frames();
  test_other_frames();
  test_validation();
  test_transport_policy();
  test_bounded_history();
  test_subscriber_registry();
  test_subscriber_scheduling_and_lag_isolation();
  return 0;
}
