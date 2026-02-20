#define DT_DRV_COMPAT zmk_behavior_raw_layer

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

extern void rawhid_send_layer(uint8_t layer, uint8_t flags);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int behavior_rawhid_layer_pressed(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    uint8_t layer = binding->param1;

    LOG_DBG("position %d layer %d", event.position, layer);
    rawhid_send_layer(layer, 0);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_rawhid_layer_released(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_rawhid_layer_driver_api = {
    .binding_pressed = behavior_rawhid_layer_pressed,
    .binding_released = behavior_rawhid_layer_released,
};

#define RAW_LAYER_INST(n)                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                               \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_rawhid_layer_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RAW_LAYER_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
