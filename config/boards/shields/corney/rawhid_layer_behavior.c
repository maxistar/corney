#include <zmk/behavior.h>
#include <zmk/keymap.h>

extern void rawhid_send_layer(uint8_t layer, uint8_t flags);

static int behavior_rawhid_layer_pressed(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event)
{
    uint8_t layer = binding->param1;

    rawhid_send_layer(layer, 0);

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_rawhid_layer_driver_api = {
    .binding_pressed = behavior_rawhid_layer_pressed,
};

BEHAVIOR_DT_INST_DEFINE(0,
    behavior_rawhid_layer_pressed,
    NULL,
    NULL,
    &behavior_rawhid_layer_driver_api
);