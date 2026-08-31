#define DT_DRV_COMPAT corney_behavior_combo_telemetry

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/util_macro.h>
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_COMBO_TEST_LOG)
#include <zephyr/sys/printk.h>
#endif

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>

#include <corney/ble_contract.h>
#include <corney/ble_events.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct combo_telemetry_config {
  uint16_t combo_id;
  struct zmk_behavior_binding behavior;
  const uint8_t *positions;
  uint8_t position_count;
};

static int combo_pressed(struct zmk_behavior_binding *binding,
                         struct zmk_behavior_binding_event event) {
  const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
  const struct combo_telemetry_config *cfg = dev->config;

#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_COMBO_EVENTS)
  corney_ble_publish_combo(cfg->combo_id, true,
                           (uint8_t)zmk_keymap_highest_layer_active(),
                           cfg->positions, cfg->position_count);
#endif
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_COMBO_TEST_LOG)
  printk("combo_telemetry: id=%u action=pressed\n", cfg->combo_id);
#endif
  return zmk_behavior_invoke_binding(&cfg->behavior, event, true);
}

static int combo_released(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
  const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
  const struct combo_telemetry_config *cfg = dev->config;

#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_COMBO_EVENTS)
  corney_ble_publish_combo(cfg->combo_id, false,
                           (uint8_t)zmk_keymap_highest_layer_active(),
                           cfg->positions, cfg->position_count);
#endif
#if IS_ENABLED(CONFIG_ZMK_KEYBOARD_HELPER_COMBO_TEST_LOG)
  printk("combo_telemetry: id=%u action=released\n", cfg->combo_id);
#endif
  return zmk_behavior_invoke_binding(&cfg->behavior, event, false);
}

static const struct behavior_driver_api combo_driver_api = {
    .binding_pressed = combo_pressed,
    .binding_released = combo_released,
};

#define COMBO_BINDING(n) ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n))
#define COMBO_POSITIONS(n) DT_INST_PROP(n, key_positions)

#define COMBO_INST(n)                                                          \
  BUILD_ASSERT(DT_INST_PROP(n, combo_id) > 0, "combo-id zero is reserved");    \
  BUILD_ASSERT(DT_INST_PROP_LEN(n, key_positions) <=                           \
                   CORNEY_BLE_MAX_COMBO_POSITIONS,                             \
               "too many combo positions");                                    \
  static const uint8_t combo_positions_##n[] = COMBO_POSITIONS(n);             \
  static const struct combo_telemetry_config combo_config_##n = {              \
      .combo_id = DT_INST_PROP(n, combo_id),                                   \
      .behavior = COMBO_BINDING(n),                                            \
      .positions = combo_positions_##n,                                        \
      .position_count = DT_INST_PROP_LEN(n, key_positions),                    \
  };                                                                           \
  BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &combo_config_##n, POST_KERNEL, \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                 \
                          &combo_driver_api);

DT_INST_FOREACH_STATUS_OKAY(COMBO_INST)

#endif
