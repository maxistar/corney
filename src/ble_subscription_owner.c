#include <errno.h>
#include <stddef.h>

#include <corney/ble_subscription_owner.h>

int corney_ble_subscription_claim(struct corney_ble_subscription_owner *state,
                                  const void *connection) {
  if (state == NULL || connection == NULL) {
    return -EINVAL;
  }
  if (state->connection == NULL) {
    state->connection = connection;
    return 0;
  }
  return state->connection == connection ? 0 : -EBUSY;
}

bool corney_ble_subscription_release(
    struct corney_ble_subscription_owner *state, const void *connection) {
  if (state == NULL || state->connection != connection) {
    return false;
  }
  state->connection = NULL;
  return true;
}

bool corney_ble_subscription_is_owner(
    const struct corney_ble_subscription_owner *state, const void *connection) {
  return state != NULL && state->connection == connection;
}
