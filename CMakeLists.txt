cmake_minimum_required(VERSION 3.20.0)

zephyr_library()
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
zephyr_library_sources_ifdef(CONFIG_ZMK_GATT_LAYER_EXPOSITION src/gatt_layer_exposition.c)
