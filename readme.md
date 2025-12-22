# A Corney Chocoflan keyboard

- body remixed from this: https://www.printables.com/model/1020389-wireless-corne-chocoflan-minimal-keyboard-case

## What's here

- `config/`: ZMK config for a split Corne on nice!nano (keymap, macros, Bluetooth bindings, west manifest).
- `body/`: printable/parametric case and plate models for the Chocoflan remix (scad, stl, step, 3mf).
- `build.yaml`: build matrix for CI (left/right halves on nice_nano@2.0.0).
- `zephyr/module.yml`: declares the repo as a board root for west builds.
- `boards/shields/`: placeholder if you want to add custom shields locally.

## Build firmware (locally)

1. Install Zephyr SDK + west per ZMK docs.
2. From the repo root, pull ZMK: `west init -l config && west update`.
3. Build each half (outputs land in `build/<side>/zephyr/zmk.uf2`):
   - Left: `west build -s zmk/app -d build/left -b nice_nano@2.0.0 -- -DSHIELD=corne_left -DZMK_CONFIG=$PWD/config`
   - Right: `west build -s zmk/app -d build/right -b nice_nano@2.0.0 -- -DSHIELD=corne_right -DZMK_CONFIG=$PWD/config`
4. Copy the corresponding UF2 to each nice!nano over USB bootloader.



