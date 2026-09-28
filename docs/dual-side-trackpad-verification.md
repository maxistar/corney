# Dual-side Cirque Trackpad Verification

Date: 2026-09-28

## Firmware topology

The release pair supports an optional Cirque Pinnacle at address `0x2a` on either or both halves:

- `corney-left-enhanced` polls a local sensor with `invert-x` and sends its events through a
  dedicated local ZMK input listener. It also retains the separate input-split proxy listener for
  the right sensor.
- `corney-right` retains the accepted polling device, `invert-y` orientation, and input-split
  endpoint.
- `settings-reset` contains neither Cirque device nor pointing listener.

The left/central release build uses the default Bluetooth name `Corney`; no
`CONFIG_ZMK_KEYBOARD_NAME` override was supplied. ZMK Studio remains disabled and the complete
Keyboard Helper capability set remains enabled.

## Automated verification

- `tests/run-host-tests.sh`: pass, including packet, wheel, button, per-side orientation, BLE
  metadata, and exact release-matrix checks.
- `tests/run-zmk-integration-tests.sh`: pass for all four fixtures, including input-split movement,
  wheel, button, disconnect release, reconnect, and synchronization events.
- `tests/verify_cirque_build.py`: pass for generated `corney_left`, `corney_right`, and
  `settings_reset` configurations and devicetrees.
- All three `nice_nano_v2` release targets build successfully without adding an artifact.

## Central resource change

Relative to the accepted Studio-free central from `remove-zmk-studio-from-corney-firmware`:

| Central build | FLASH | RAM | UF2 size |
| --- | ---: | ---: | ---: |
| Studio-free, right sensor only | 243,864 B | 59,836 B | 487,936 B |
| Optional left and right sensors | 247,916 B | 60,036 B | 496,128 B |
| Change | +4,052 B | +200 B | +8,192 B |

## Candidate artifacts

The default-name release candidates are stored under
`.artifacts/restore-corney-left-trackpad/`:

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-left-enhanced.uf2` | 496128 bytes | `fedd853f9ae453237221466a126c01cd26dcd3ffa5e9861498a8bbda25adb838` |
| `corney-right.uf2` | 355840 bytes | `f7b7a74931fbe5ddcad91d0888dd13a3f5954c8e8d1eb1066bdf61b1280a6185` |
| `settings-reset.uf2` | 92672 bytes | `fd3072a56b359a56b08ad0c010b77ebe516846fc59ca21aa5c1059518a082d60` |

The right and settings-reset images are byte-identical to the preceding accepted release; only the
left central changed.

## Physical acceptance

User-confirmed on 2026-09-28:

- On the available left-sensor keyboard, X/Y direction, scrolling, primary tap, USB and BLE
  pointing, key-driven idle resume, ordinary typing, combos, layers, and Keyboard Helper work as
  expected.
- The retained right-sensor path, split reconnect, right-local-key resume, right-source release,
  and sensorless behavior remain confirmed by the preceding right-peripheral acceptance and were
  reported as working as expected for the remaining scenarios.
- No new battery, responsiveness, or throughput limitation was observed during the reported tests.

A keyboard with physical sensors installed on both halves was not available. Dual-sensor
coexistence and simultaneous relative input on one split pair therefore remain unverified hardware
scenarios even though both generated paths compile together in the central image. This limitation
was explicitly accepted for archiving.

Movement, scrolling, and non-overlapping taps are supported from either sensor. Simultaneously
holding the same logical mouse button from both sensors is outside the accepted contract because
the pinned ZMK input listener does not source-count button ownership; this unsupported gesture was
not physically tested.
