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
- `build.yaml`: build matrix for CI (direct central, BLE halves, USB dongle, and reset firmware on `nice_nano_v2`).
- `zephyr/module.yml`: declares the repo as a ZMK module, including the Corney shield root and
  the custom GATT features.
- `docs/gatt-layer-exposition.md`: UUIDs, data format, and build notes for the BLE layer
  characteristic.
- `docs/keyboard-helper-ble-v1.md`: implementation, security, queue, and build notes for the
  versioned Keyboard Helper event extension.

## Clone

```bash
git clone https://github.com/maxistar/corney.git
cd corney
```

## Prerequisites

- Zephyr SDK and `west` already installed ([ZMK setup guide](https://zmk.dev/docs/development/setup))

## Build firmware (locally)

1. From the repo root, pull ZMK: `west init -l config && west update`.
2. Build the direct topology (outputs land in `build/<side>/zephyr/zmk.uf2`):
   - Left (enhanced): `west build -p -s zmk/app -d build/left -b nice_nano_v2 -- -DSHIELD=corney_left -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD -DCONFIG_ZMK_STUDIO=n -DCONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y`
   - Right: `west build -p -s zmk/app -d build/right -b nice_nano_v2 -- -DSHIELD=corney_right -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD`
3. Or build the dongle topology:
   - Left peripheral: `west build -p -s zmk/app -d build/left-peripheral -b nice_nano_v2 -- -DSHIELD=corney_left_peripheral -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD`
   - Right peripheral: use the same `corney_right` command and artifact as direct mode.
   - USB dongle: `west build -p -s zmk/app -d build/dongle -b nice_nano_v2 -- -DSHIELD=corney_dongle -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD -DCONFIG_ZMK_STUDIO=n -DCONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y`
4. Copy the corresponding UF2 to each nice!nano over USB bootloader.

`corney-left-enhanced` is the single supported left-central image for direct BLE operation. It
provides the ordinary keyboard HID behavior together with Keyboard Helper telemetry. Pair it with
the universal `corney-right` image, which supports right halves both with and without the physical
trackpad sensor.

The optional dongle topology uses `corney-left-peripheral` and the same `corney-right` image as two
BLE peripherals. `corney-usb-dongle` is their keyless central and sends keyboard and pointing HID
to the computer over USB. It keeps the full Keyboard Helper GATT service available over a separate
encrypted BLE connection. Both host-facing central images use the default name `Corney`.

ZMK Studio is intentionally disabled in the release firmware. The active layout is compiled from
`config/corney.keymap`; change that file and rebuild the left image to edit the layout. Previously
saved Studio overrides are not applied. The full Keyboard Helper event and diagnostics service
remains enabled.

## Build firmware with a custom Bluetooth name

The default Bluetooth device name is `Corney`. To override it, pass
`CONFIG_ZMK_KEYBOARD_NAME` when building a host-facing central.

Local build examples:

- Left (enhanced): `west build -p -s zmk/app -d build/left -b nice_nano_v2 -- -DSHIELD=corney_left -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD -DCONFIG_ZMK_STUDIO=n -DCONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y -DCONFIG_ZMK_KEYBOARD_NAME=\"CorneyMX\"`
- USB dongle: `west build -p -s zmk/app -d build/dongle -b nice_nano_v2 -- -DSHIELD=corney_dongle -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD -DCONFIG_ZMK_STUDIO=n -DCONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y -DCONFIG_ZMK_KEYBOARD_NAME=\"CorneyMX\"`
- Right: `west build -p -s zmk/app -d build/right -b nice_nano_v2 -- -DSHIELD=corney_right -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD`

Do not apply the custom name override to either peripheral image. Only the active central is
host-facing.

## Cirque trackpad

Both halves support an optional Cirque Pinnacle trackpad over their own Pro Micro I2C pins. Each
sensor uses address `0x2a`, does not connect its data-ready (`DR`) signal, and replaces rather than
shares the former OLED position. The same `corney-left-enhanced` and `corney-right` images support
four physical arrangements: no sensor, left only, right only, or one sensor on each half.

Because DR is absent, each controller with an installed sensor reads its status every 8 ms while
active. The left central consumes its local sensor directly. The right half forwards relative
movement, primary tap, and wheel packets over ZMK input-split to a separate central proxy listener.
Both paths produce ordinary USB or BLE HID mouse reports; Keyboard Helper remains central-owned and
does not carry pointer packets.

In dongle mode, both sensors are remote. The right endpoint retains input-split register `0`; the
left peripheral uses register `1`. The dongle has matching proxy listeners and no physical sensor
or matrix of its own. Button state is tracked per proxy. Because pinned ZMK v0.3.0 does not expose
the disconnected split connection's input-split identity, any split disconnect conservatively
clears retained remote button state; later events from a still-connected half continue normally.

The original left installation uses `invert-x`. The accepted right installation is rotated 180
degrees relative to its first prototype and uses `invert-y` without `invert-x`. These settings make
physical finger direction match cursor direction on each assembled half.

If either image boots without its optional sensor, initialization fails once and periodic polling
is not started on that controller. Matrix scanning, split operation, and an installed sensor on the
other half remain independent. The pinned ZMK baseline does not release input-split buttons
automatically on disconnect, so the left firmware tracks only right-proxy button state and performs
a bounded safety release when that split connection disappears. Local left buttons are not part of
that disconnect state.

Continuous polling costs additional battery power on every sensor-equipped half. During system
suspend the corresponding polling work stops; trackpad touch alone cannot wake that controller
without DR. Wake the left sensor with a left-local key and the right sensor with a right-local key or
another wake source on the same controller. The right pointing path also waits for split reconnect.

Movement, scrolling, and ordinary non-overlapping taps may be used from either sensor. The pinned
ZMK input listener does not source-count a simultaneous hold of the same mouse button from two
sensors, so that specific dual-button gesture is unsupported.

The explicitly named Choc printable entry points are
`body/Choc_Version/right_touchpad_body.scad` and
`body/Choc_Version/right_touchpad_cover.scad`; use
`body/Choc_Version/right_touchpad_assembly.scad` to inspect controller, reset, sensor, and wiring
clearances. Hardware acceptance for this topology is tracked in
`docs/right-peripheral-trackpad-verification.md`.

## CI/CD

GitHub Actions is the primary CI/CD pipeline. It runs portable protocol/metadata tests, native ZMK
integration tests, and builds `corney-left-enhanced`, `corney-left-peripheral`, `corney-right`,
`corney-usb-dongle`, and `settings-reset` firmware artifacts. A manual workflow run can override
the Bluetooth name for either central image only.

The GitLab CI configuration is retained as an equivalent alternative for a future GitLab mirror.
Local Bluetooth name overrides remain a CMake option so the right/peripheral image cannot
accidentally receive a host-facing name.
