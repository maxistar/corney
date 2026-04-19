# GATT layer number exposition

This document defines the custom GATT service and characteristic used to expose the active layer number.
The feature is implemented as part of this repository's external ZMK module and is enabled by
default for Corney shield builds through shield `Kconfig.defconfig` defaults.

## UUIDs
- Service UUID: `715d81e1-377d-4f26-a678-a506675d99ec`
- Characteristic UUID (Layer Number): `e87c518a-f323-4dd7-9dd5-0991add1c01b`

## Data format
- Value: unsigned 8-bit integer
- Endianness: not applicable (single byte)
- Semantics: zero-based active layer number, representing the highest active layer

## Properties
- Read
- Notify

## Build integration
- GitHub Actions user-config builds discover the service through this repo's `zephyr/module.yml`.
- Local `west build` commands should include `-DZMK_EXTRA_MODULES=$PWD` so the module sources are
  compiled alongside the Corney config.
- The feature can be disabled for a build by setting `CONFIG_ZMK_GATT_LAYER_EXPOSITION=n` in an
  override `.conf` file or via an explicit CMake config override.
