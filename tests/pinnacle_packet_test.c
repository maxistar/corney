#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <corney/pinnacle_packet.h>

static void test_status(void) {
  assert(!corney_pinnacle_status_has_data(0x00));
  assert(corney_pinnacle_status_has_data(0x04));
  assert(corney_pinnacle_status_has_data(0x0c));
  assert(!corney_pinnacle_status_has_data(0xff));
}

static void test_positive_and_zero_axes(void) {
  const uint8_t packet[] = {CORNEY_PINNACLE_BUTTON_PRIMARY, 0x7f, 0x00, 0x01};
  struct corney_pinnacle_relative_sample sample;

  assert(corney_pinnacle_decode_relative(packet, sizeof(packet), &sample) == 0);
  assert(sample.x == 127);
  assert(sample.y == 0);
  assert(sample.wheel == 1);
  assert(sample.buttons == CORNEY_PINNACLE_BUTTON_PRIMARY);
}

static void test_negative_and_boundary_axes(void) {
  const uint8_t negative_one[] = {0x30, 0xff, 0xff, 0xff};
  const uint8_t negative_256[] = {0x30, 0x00, 0x00, 0x00};
  const uint8_t positive_255[] = {0x00, 0xff, 0xff, 0x7f};
  struct corney_pinnacle_relative_sample sample;

  assert(corney_pinnacle_decode_relative(negative_one, sizeof(negative_one),
                                         &sample) == 0);
  assert(sample.x == -1);
  assert(sample.y == -1);
  assert(sample.wheel == -1);

  assert(corney_pinnacle_decode_relative(negative_256, sizeof(negative_256),
                                         &sample) == 0);
  assert(sample.x == -256);
  assert(sample.y == -256);

  assert(corney_pinnacle_decode_relative(positive_255, sizeof(positive_255),
                                         &sample) == 0);
  assert(sample.x == 255);
  assert(sample.y == 255);
  assert(sample.wheel == 127);
}

static void test_button_transitions(void) {
  assert(corney_pinnacle_button_changes(0, 0) == 0);
  assert(corney_pinnacle_button_changes(0, CORNEY_PINNACLE_BUTTON_PRIMARY) ==
         CORNEY_PINNACLE_BUTTON_PRIMARY);
  assert(corney_pinnacle_button_changes(CORNEY_PINNACLE_BUTTON_PRIMARY,
                                        CORNEY_PINNACLE_BUTTON_PRIMARY) == 0);
  assert(corney_pinnacle_button_changes(CORNEY_PINNACLE_BUTTON_PRIMARY, 0) ==
         CORNEY_PINNACLE_BUTTON_PRIMARY);
  assert(corney_pinnacle_button_changes(0xff, 0) ==
         CORNEY_PINNACLE_BUTTON_MASK);
}

static void test_invalid_packets(void) {
  const uint8_t packet[CORNEY_PINNACLE_RELATIVE_PACKET_SIZE] = {0};
  struct corney_pinnacle_relative_sample sample;

  assert(corney_pinnacle_decode_relative(NULL, sizeof(packet), &sample) ==
         -EINVAL);
  assert(corney_pinnacle_decode_relative(packet, sizeof(packet), NULL) ==
         -EINVAL);
  assert(corney_pinnacle_decode_relative(packet, sizeof(packet) - 1, &sample) ==
         -EINVAL);
  assert(corney_pinnacle_decode_relative(packet, sizeof(packet) + 1, &sample) ==
         -EINVAL);
}

int main(void) {
  test_status();
  test_positive_and_zero_axes();
  test_negative_and_boundary_axes();
  test_button_transitions();
  test_invalid_packets();
  return 0;
}
