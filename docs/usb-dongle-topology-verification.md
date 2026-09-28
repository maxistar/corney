# Corney USB Dongle Topology Verification

Date: 2026-09-28

## Firmware roles

- `corney-left-enhanced`: existing direct central with local left Cirque, right input-split proxy,
  USB/BLE HID, and Keyboard Helper BLE service.
- `corney-left-peripheral`: BLE left half with matrix scanning and optional Cirque forwarded through
  input-split register `1`.
- `corney-right`: universal BLE right half with optional Cirque forwarded through the retained
  input-split register `0`.
- `corney-usb-dongle`: keyless central with two BLE peripheral slots, two pointing proxies, USB
  HID, and Keyboard Helper BLE service.
- `settings-reset`: bond-reset utility.

Both central targets use the default device name `Corney`. ZMK Studio remains disabled. Keyboard
Helper continues to use encrypted BLE GATT even while the dongle sends HID to the computer over
USB. The dongle does not report the battery level of either half.

## Entering dongle mode

Changing the central invalidates the existing split bonds. Do not try to reuse bonds from direct
mode.

1. Flash `settings-reset` to the future dongle, left half, and right half; allow each device to boot.
2. Flash `corney-usb-dongle` to the dongle.
3. Flash `corney-left-peripheral` to the left half.
4. Flash the unchanged `corney-right` artifact to the right half.
5. Power the dongle from the computer over USB and power both halves. Allow both split bonds to be
   established before testing.

## Returning to direct mode

1. Flash `settings-reset` to the left and right halves; reset the dongle too before its next use.
2. Flash `corney-left-enhanced` to the left half.
3. Flash the same `corney-right` artifact to the right half.
4. Pair the `Corney` central to the intended BLE hosts again.

## Automated acceptance

- [x] Host tests pass, including exact five-artifact matrix checks.
- [x] Native ZMK integration tests pass, including two input-split registers, independent retained
  button state, bounded cleanup, and continued post-cleanup input.
- [x] All five `nice_nano_v2` targets build on pinned ZMK `v0.3.0`.
- [x] Generated configurations and devicetrees pass `tests/verify_cirque_build.py`.
- [x] Candidate UF2 files, hashes, and generated configuration evidence are retained.

The candidate files are stored under `.artifacts/add-corney-usb-dongle-topology/`; matching
resolved `.config` and `zephyr.dts` files are retained in its `config/` and `devicetree/`
subdirectories.

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-left-enhanced.uf2` | 496128 bytes | `7aba0ccd18874ecdf3829aa3a69ed6db863d40a87026ecf3969bad7ef5ce696c` |
| `corney-left-peripheral.uf2` | 355840 bytes | `51069e75b64495508b1ecaff860dee4f62a85456fa7d382ec870c45f81f2396b` |
| `corney-right.uf2` | 355840 bytes | `f7b7a74931fbe5ddcad91d0888dd13a3f5954c8e8d1eb1066bdf61b1280a6185` |
| `corney-usb-dongle.uf2` | 485888 bytes | `d8577d60330acd78af3eca2b65f170738285cfd2bf35ec7b99f1e02ebd1144f0` |
| `settings-reset.uf2` | 92672 bytes | `fd3072a56b359a56b08ad0c010b77ebe516846fc59ca21aa5c1059518a082d60` |

## Physical acceptance

- [x] Keys, combos, and layers from each half work independently and together over dongle USB HID.
- [x] Each half can be power-cycled without disabling the other, reconnects, and leaves no stuck
  key or pointing button.
- [x] Every available left/right Cirque path is checked for movement, orientation, tap, supported
  wheel input, key-driven idle resume, reconnect, and sensorless operation.
- [x] An encrypted BLE client observes Keyboard Helper capabilities and representative key, combo,
  layer, reconnect, and two-subscriber behavior while USB HID is active.
- [ ] The direct `corney-left-enhanced` plus `corney-right` topology still passes its accepted
  no-dongle regression checks after resetting bonds. This check was explicitly deferred by the
  user on 2026-09-28 to avoid reflashing the currently accepted dongle topology.
- [x] Simultaneous operation of two installed compatible Cirque sensors is explicitly recorded as
  **pending**, not passed: a two-sensor physical assembly is not currently available.

On 2026-09-28 the user flashed all three dongle-topology devices and confirmed operation of the
dongle and all existing functions. The direct-topology rollback check remains outstanding and was
explicitly deferred because it requires resetting bonds and reflashing the left half from
`corney-left-peripheral` to `corney-left-enhanced` (followed by re-pairing); the current confirmation
covered the three-device dongle topology.
