#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <corney/dongle_power_status.h>

static void assert_text(const struct corney_dongle_power_status *status,
                        const char *expected) {
  char actual[CORNEY_DONGLE_POWER_TEXT_SIZE];
  int length =
      corney_dongle_power_status_format(status, actual, sizeof(actual));

  assert(length >= 0);
  assert((size_t)length < sizeof(actual));
  assert(strcmp(actual, expected) == 0);
}

static void test_initial_and_independent_updates(void) {
  struct corney_dongle_power_status status;

  corney_dongle_power_status_init(&status);
  assert_text(&status, "-- --");

  assert(corney_dongle_power_status_set_half(&status, 0U, 78U) == 0);
  assert(status.halves[0].available);
  assert(!status.halves[1].available);
  assert_text(&status, "78% --");

  assert(corney_dongle_power_status_set_half(&status, 1U, 64U) == 0);
  assert_text(&status, "78% 64%");
}

static void test_bounds_and_disconnect(void) {
  struct corney_dongle_power_status status;

  corney_dongle_power_status_init(&status);
  assert(corney_dongle_power_status_set_half(&status, 0U, UINT8_MAX) == 0);
  assert(status.halves[0].level == 100U);
  assert(corney_dongle_power_status_set_half(&status, 1U, 1U) == 0);
  assert(corney_dongle_power_status_set_half(&status, 2U, 50U) == -EINVAL);
  assert_text(&status, "100% 1%");

  assert(corney_dongle_power_status_set_half(&status, 0U, 0U) == 0);
  assert(!status.halves[0].available);
  assert_text(&status, "-- 1%");
}

static void test_reversed_slot_arrival_and_widest_pair(void) {
  struct corney_dongle_power_status status;

  corney_dongle_power_status_init(&status);
  assert(corney_dongle_power_status_set_half(&status, 1U, 56U) == 0);
  assert_text(&status, "-- 56%");
  assert(corney_dongle_power_status_set_half(&status, 0U, 78U) == 0);
  assert_text(&status, "78% 56%");

  assert(corney_dongle_power_status_set_half(&status, 0U, 100U) == 0);
  assert(corney_dongle_power_status_set_half(&status, 1U, 100U) == 0);
  assert_text(&status, "100% 100%");
}

static void test_layout_bands(void) {
  assert(CORNEY_DONGLE_STOCK_TOP_HEIGHT_PX <= CORNEY_DONGLE_REMOTE_ROW_Y_PX);
  assert(CORNEY_DONGLE_REMOTE_ROW_Y_PX + CORNEY_DONGLE_REMOTE_ROW_HEIGHT_PX <=
         CORNEY_DONGLE_DISPLAY_HEIGHT_PX -
             CORNEY_DONGLE_STOCK_BOTTOM_HEIGHT_PX);
  assert(CORNEY_DONGLE_REMOTE_ROW_WIDTH_PX <= CORNEY_DONGLE_DISPLAY_WIDTH_PX);
}

int main(void) {
  test_initial_and_independent_updates();
  test_bounds_and_disconnect();
  test_reversed_slot_arrival_and_widest_pair();
  test_layout_bands();
  return 0;
}
