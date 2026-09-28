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

  corney_dongle_power_status_init(&status, 42U, false);
  assert_text(&status, "Half 1   --\nHalf 2   --\nDongle   42% BAT");

  assert(corney_dongle_power_status_set_half(&status, 0U, 78U) == 0);
  assert(status.halves[0].available);
  assert(!status.halves[1].available);
  assert_text(&status, "Half 1   78%\nHalf 2   --\nDongle   42% BAT");

  assert(corney_dongle_power_status_set_half(&status, 1U, 64U) == 0);
  assert_text(&status, "Half 1   78%\nHalf 2   64%\nDongle   42% BAT");
}

static void test_bounds_disconnect_and_usb(void) {
  struct corney_dongle_power_status status;

  corney_dongle_power_status_init(&status, UINT8_MAX, true);
  assert(status.dongle_level == 100U);
  assert(corney_dongle_power_status_set_half(&status, 0U, UINT8_MAX) == 0);
  assert(status.halves[0].level == 100U);
  assert(corney_dongle_power_status_set_half(&status, 1U, 1U) == 0);
  assert(corney_dongle_power_status_set_half(&status, 2U, 50U) == -EINVAL);
  assert_text(&status, "Half 1  100%\nHalf 2    1%\nDongle  100% USB");

  assert(corney_dongle_power_status_set_half(&status, 0U, 0U) == 0);
  assert(!status.halves[0].available);
  corney_dongle_power_status_set_local(&status, 9U);
  corney_dongle_power_status_set_usb(&status, false);
  assert_text(&status, "Half 1   --\nHalf 2    1%\nDongle    9% BAT");
}

int main(void) {
  test_initial_and_independent_updates();
  test_bounds_disconnect_and_usb();
  return 0;
}
