#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/display/status_screen.h>
#include <zmk/display/widgets/battery_status.h>
#include <zmk/display/widgets/output_status.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/split/central.h>

#include <corney/dongle_power_status.h>

static struct zmk_widget_battery_status battery_status_widget;
static struct zmk_widget_output_status output_status_widget;
static lv_obj_t *layer_name_label;
static lv_obj_t *remote_power_label;
static struct corney_dongle_power_status power_status;
static bool power_status_initialized;

struct corney_dongle_layer_status {
  const char *name;
};

static void layer_status_update_cb(struct corney_dongle_layer_status status) {
  lv_label_set_text(layer_name_label, status.name == NULL ? "" : status.name);
}

static struct corney_dongle_layer_status
layer_status_get_state(const zmk_event_t *event) {
  (void)event;
  zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
  zmk_keymap_layer_id_t layer = zmk_keymap_layer_index_to_id(index);

  return (struct corney_dongle_layer_status){
      .name = zmk_keymap_layer_name(layer),
  };
}

ZMK_DISPLAY_WIDGET_LISTENER(corney_dongle_layer_listener,
                            struct corney_dongle_layer_status,
                            layer_status_update_cb, layer_status_get_state)

ZMK_SUBSCRIPTION(corney_dongle_layer_listener, zmk_layer_state_changed);

static void initialize_power_status(void) {
  if (power_status_initialized) {
    return;
  }

  corney_dongle_power_status_init(&power_status);

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

  const struct zmk_peripheral_battery_state_changed *remote =
      as_zmk_peripheral_battery_state_changed(event);
  if (remote != NULL) {
    corney_dongle_power_status_set_half(&power_status, remote->source,
                                        remote->state_of_charge);
    return power_status;
  }

  return power_status;
}

static void power_status_update_cb(struct corney_dongle_power_status status) {
  char text[CORNEY_DONGLE_POWER_TEXT_SIZE];

  corney_dongle_power_status_format(&status, text, sizeof(text));
  lv_label_set_text(remote_power_label, text);
}

ZMK_DISPLAY_WIDGET_LISTENER(corney_dongle_power_listener,
                            struct corney_dongle_power_status,
                            power_status_update_cb, power_status_get_state)

ZMK_SUBSCRIPTION(corney_dongle_power_listener,
                 zmk_peripheral_battery_state_changed);

lv_obj_t *zmk_display_status_screen(void) {
  lv_obj_t *screen = lv_obj_create(NULL);

  zmk_widget_output_status_init(&output_status_widget, screen);
  lv_obj_align(zmk_widget_output_status_obj(&output_status_widget),
               LV_ALIGN_TOP_LEFT, 0, 0);

  zmk_widget_battery_status_init(&battery_status_widget, screen);
  lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget),
               LV_ALIGN_TOP_RIGHT, 0, 0);

  lv_obj_t *layer_icon_label = lv_label_create(screen);
  lv_label_set_text(layer_icon_label, LV_SYMBOL_KEYBOARD);
  lv_obj_set_width(layer_icon_label, CORNEY_DONGLE_LAYER_ICON_WIDTH_PX);
  lv_obj_set_style_text_font(layer_icon_label, lv_theme_get_font_small(screen),
                             LV_PART_MAIN);
  lv_obj_align(layer_icon_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

  layer_name_label = lv_label_create(screen);
  lv_obj_set_width(layer_name_label, CORNEY_DONGLE_LAYER_NAME_WIDTH_PX);
  lv_label_set_long_mode(layer_name_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_anim_speed(layer_name_label,
                              CORNEY_DONGLE_LAYER_SCROLL_SPEED_PX_PER_SEC,
                              LV_PART_MAIN);
  lv_obj_set_style_text_font(layer_name_label, lv_theme_get_font_small(screen),
                             LV_PART_MAIN);
  lv_obj_align(layer_name_label, LV_ALIGN_BOTTOM_LEFT,
               CORNEY_DONGLE_LAYER_NAME_X_PX, 0);

  remote_power_label = lv_label_create(screen);
  lv_obj_set_width(remote_power_label, CORNEY_DONGLE_REMOTE_ROW_WIDTH_PX);
  lv_label_set_long_mode(remote_power_label, LV_LABEL_LONG_CLIP);
  lv_obj_set_style_text_font(remote_power_label, &lv_font_montserrat_12,
                             LV_PART_MAIN);
  lv_obj_set_style_text_align(remote_power_label, LV_TEXT_ALIGN_RIGHT,
                              LV_PART_MAIN);
  lv_obj_align(remote_power_label, LV_ALIGN_TOP_RIGHT, 0,
               CORNEY_DONGLE_REMOTE_ROW_Y_PX);

  corney_dongle_layer_listener_init();
  corney_dongle_power_listener_init();

  return screen;
}
