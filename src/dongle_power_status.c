#include <errno.h>
#include <stdio.h>

#include <corney/dongle_power_status.h>

static uint8_t bounded_level(uint8_t level) {
  return level > 100U ? 100U : level;
}

void corney_dongle_power_status_init(struct corney_dongle_power_status *status,
                                     uint8_t dongle_level, bool usb_powered) {
  status->dongle_level = bounded_level(dongle_level);
  status->usb_powered = usb_powered;

  for (size_t i = 0U; i < CORNEY_DONGLE_HALF_COUNT; i++) {
    status->halves[i].level = 0U;
    status->halves[i].available = false;
  }
}

void corney_dongle_power_status_set_local(
    struct corney_dongle_power_status *status, uint8_t level) {
  status->dongle_level = bounded_level(level);
}

void corney_dongle_power_status_set_usb(
    struct corney_dongle_power_status *status, bool powered) {
  status->usb_powered = powered;
}

int corney_dongle_power_status_set_half(
    struct corney_dongle_power_status *status, uint8_t source, uint8_t level) {
  if (source >= CORNEY_DONGLE_HALF_COUNT) {
    return -EINVAL;
  }

  status->halves[source].level = bounded_level(level);
  status->halves[source].available = level != 0U;
  return 0;
}

static int format_half(char *buffer, size_t buffer_size, size_t half_number,
                       const struct corney_dongle_half_power *half) {
  if (!half->available) {
    return snprintf(buffer, buffer_size, "Half %zu   --", half_number);
  }

  return snprintf(buffer, buffer_size, "Half %zu  %3u%%", half_number,
                  half->level);
}

int corney_dongle_power_status_format(
    const struct corney_dongle_power_status *status, char *buffer,
    size_t buffer_size) {
  char first[16];
  char second[16];

  format_half(first, sizeof(first), 1U, &status->halves[0]);
  format_half(second, sizeof(second), 2U, &status->halves[1]);

  return snprintf(buffer, buffer_size, "%s\n%s\nDongle  %3u%% %s", first,
                  second, status->dongle_level,
                  status->usb_powered ? "USB" : "BAT");
}
