#include <lvgl.h>

#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/display/status_screen.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/split/central.h>
#include <zmk/usb.h>

#include <corney/dongle_power_status.h>

static lv_obj_t *power_label;
static struct corney_dongle_power_status power_status;
static bool power_status_initialized;

static void initialize_power_status(void) {
  if (power_status_initialized) {
    return;
  }

  corney_dongle_power_status_init(&power_status, zmk_battery_state_of_charge(),
                                  zmk_usb_is_powered());

  for (uint8_t source = 0U; source < CORNEY_DONGLE_HALF_COUNT; source++) {
    uint8_t level = 0U;
    if (zmk_split_central_get_peripheral_battery_level(source, &level) == 0) {
      corney_dongle_power_status_set_half(&power_status, source, level);
    }
  }

  power_status_initialized = true;
}

static struct corney_dongle_power_status
power_status_get_state(const zmk_event_t *event) {
  initialize_power_status();

  const struct zmk_battery_state_changed *local =
      as_zmk_battery_state_changed(event);
  if (local != NULL) {
    corney_dongle_power_status_set_local(&power_status, local->state_of_charge);
    return power_status;
  }

  const struct zmk_peripheral_battery_state_changed *remote =
      as_zmk_peripheral_battery_state_changed(event);
  if (remote != NULL) {
    corney_dongle_power_status_set_half(&power_status, remote->source,
                                        remote->state_of_charge);
    return power_status;
  }

  if (as_zmk_usb_conn_state_changed(event) != NULL) {
    corney_dongle_power_status_set_usb(&power_status, zmk_usb_is_powered());
  }

  return power_status;
}

static void power_status_update_cb(struct corney_dongle_power_status status) {
  char text[CORNEY_DONGLE_POWER_TEXT_SIZE];

  corney_dongle_power_status_format(&status, text, sizeof(text));
  lv_label_set_text(power_label, text);
}

ZMK_DISPLAY_WIDGET_LISTENER(corney_dongle_power_listener,
                            struct corney_dongle_power_status,
                            power_status_update_cb, power_status_get_state)

ZMK_SUBSCRIPTION(corney_dongle_power_listener, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(corney_dongle_power_listener,
                 zmk_peripheral_battery_state_changed);
ZMK_SUBSCRIPTION(corney_dongle_power_listener, zmk_usb_conn_state_changed);

lv_obj_t *zmk_display_status_screen(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  power_label = lv_label_create(screen);
  lv_obj_set_style_text_font(power_label, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_align(power_label, LV_ALIGN_CENTER, 0, 0);

  corney_dongle_power_listener_init();

  return screen;
}
