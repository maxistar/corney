#pragma once

#include <stdbool.h>

struct corney_ble_subscription_owner {
  const void *connection;
};

int corney_ble_subscription_claim(struct corney_ble_subscription_owner *state,
                                  const void *connection);
bool corney_ble_subscription_release(
    struct corney_ble_subscription_owner *state, const void *connection);
bool corney_ble_subscription_is_owner(
    const struct corney_ble_subscription_owner *state, const void *connection);
