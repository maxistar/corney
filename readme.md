# Corney - Corney Chocoflan and Classical Corne keyboard

![](docs/corne.jpg)

![](docs/cornemx.jpg)

- body remixed from this: https://www.printables.com/model/1020389-wireless-corne-chocoflan-minimal-keyboard-case



## Related Projects

- Dacty: https://github.com/maxistar/dacty

## Layout Helper

Use Keyboard Layout Helper to inspect and tune layers visually:
https://projects.maxistar.me/keyboard_helper/

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
   - Left: `west build -p -s zmk/app -d build/left -b nice_nano@2.0.0 -- -DSHIELD=corney_left -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD`
   - Right: `west build -p -s zmk/app -d build/right -b nice_nano@2.0.0 -- -DSHIELD=corney_right -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD`
4. Copy the corresponding UF2 to each nice!nano over USB bootloader.

## Build firmware with a custom Bluetooth name

The default Bluetooth device name is `Corney`. To override it, pass
`CONFIG_ZMK_KEYBOARD_NAME` when building the left half.

Local build examples:

- Left: `west build -p -s zmk/app -d build/left -b nice_nano@2.0.0 -- -DSHIELD=corney_left -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD -DCONFIG_ZMK_KEYBOARD_NAME=\"CorneyMX\"`
- Right: `west build -p -s zmk/app -d build/right -b nice_nano@2.0.0 -- -DSHIELD=corney_right -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD`

Do not apply the custom name override to the right half. The left half is the central, host-paired side, and the right half should be built with its default configuration.

## CI/CD builds with a custom Bluetooth name

The GitHub Actions build workflow accepts an optional `keyboard_name` input when started via
`workflow_dispatch`.

1. Open the `Build ZMK firmware` workflow in GitHub Actions.
2. Click `Run workflow`.
3. Set `keyboard_name` to the Bluetooth name you want, for example `My Corney`.
4. Run the workflow and download the generated firmware artifacts.

When `keyboard_name` is provided, CI applies it only to the `corney_left` build. The `corney_right` build remains unchanged.

If `keyboard_name` is left empty, CI uses the repo default name `Corney`.
