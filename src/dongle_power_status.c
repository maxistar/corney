#include <errno.h>
#include <stdio.h>

#include <corney/dongle_power_status.h>

static uint8_t bounded_level(uint8_t level) {
  return level > 100U ? 100U : level;
}

void corney_dongle_power_status_init(
    struct corney_dongle_power_status *status) {
  for (size_t i = 0U; i < CORNEY_DONGLE_HALF_COUNT; i++) {
    status->halves[i].level = 0U;
    status->halves[i].available = false;
  }
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

static int format_half(char *buffer, size_t buffer_size,
                       const struct corney_dongle_half_power *half) {
  if (!half->available) {
    return snprintf(buffer, buffer_size, "--");
  }

  return snprintf(buffer, buffer_size, "%u%%", half->level);
}

int corney_dongle_power_status_format(
    const struct corney_dongle_power_status *status, char *buffer,
    size_t buffer_size) {
  char first[5];
  char second[5];

  format_half(first, sizeof(first), &status->halves[0]);
  format_half(second, sizeof(second), &status->halves[1]);

  return snprintf(buffer, buffer_size, "%s %s", first, second);
}
