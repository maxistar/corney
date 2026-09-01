cmake_minimum_required(VERSION 3.20.0)

zephyr_library()
zephyr_include_directories(include ${APPLICATION_SOURCE_DIR}/include)

zephyr_library_sources(src/behavior_combo_telemetry.c)
zephyr_library_sources(src/ble_frame_history.c)
zephyr_library_sources(src/ble_subscriber_registry.c)
zephyr_library_sources(src/ble_transport_policy.c)
zephyr_library_sources_ifdef(CONFIG_ZMK_GATT_LAYER_EXPOSITION
  src/ble_contract.c
  src/gatt_service.c
  src/layer_state.c
)
zephyr_library_sources_ifdef(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION
  src/ble_transport.c
)

if(CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION OR CONFIG_ZMK_KEYBOARD_HELPER_EVENT_TEST_LOG)
  # Position telemetry must observe the original event before ZMK's combo
  # listener captures it. External Zephyr module libraries are linked after
  # the ZMK app library, so prepend this one listener source to app instead.
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ble_event_sources.c)
endif()
