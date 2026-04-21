# A Corney Chocoflan keyboard

- body remixed from this: https://www.printables.com/model/1020389-wireless-corne-chocoflan-minimal-keyboard-case

## What's here

- `config/`: ZMK config for a split Corne on nice!nano (keymap, macros, Bluetooth bindings, west manifest).
- `body/`: printable/parametric case and plate models for the Chocoflan remix (scad, stl, step, 3mf).
- `build.yaml`: build matrix for CI (left/right halves on nice_nano@2.0.0).
- `zephyr/module.yml`: declares the repo as a ZMK module, including the Corney shield root and
  the custom GATT layer exposition feature.
- `docs/gatt-layer-exposition.md`: UUIDs, data format, and build notes for the BLE layer
  characteristic.

## Clone

```bash
git clone https://github.com/maxistar/corney.git
cd corney
```

## Prerequisites

- Zephyr SDK and `west` already installed ([ZMK setup guide](https://zmk.dev/docs/development/setup))

## Build firmware (locally)

1. From the repo root, pull ZMK: `west init -l config && west update`.
3. Build each half (outputs land in `build/<side>/zephyr/zmk.uf2`):
   - Left: `west build -s zmk/app -d build/left -b nice_nano@2.0.0 -- -DSHIELD=corney_left -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD`
   - Right: `west build -s zmk/app -d build/right -b nice_nano@2.0.0 -- -DSHIELD=corney_right -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD`
4. Copy the corresponding UF2 to each nice!nano over USB bootloader.

