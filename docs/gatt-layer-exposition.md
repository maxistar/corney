# GATT layer control

This document defines the custom GATT service and characteristic used to observe and select
Corney's active ZMK layer. The external module enables the service by default on the central
(`corney_left`) half. The peripheral half does not expose or execute this service.

## UUIDs

- Service UUID: `b34a0001-e782-4706-8f9c-6c056c416507`
- Layer characteristic UUID: `b34a0002-e782-4706-8f9c-6c056c416507`

These values match `layout_corney.json`.

## Value format

- Exactly four bytes.
- Signed 32-bit integer in little-endian order (`int32-le`).
- Zero-based ZMK layer index.
- Reads and notifications contain the highest currently active ZMK layer.
- Writes request `zmk_keymap_layer_to(index)`; they do not directly replace the cached value.

Examples for the current 19-layer keymap:

| Layer | Raw hexadecimal value |
| --- | --- |
| 0 | `00 00 00 00` |
| 1 | `01 00 00 00` |
| 18 | `12 00 00 00` |

The valid upper bound is compiled into the firmware as `ZMK_KEYMAP_LAYERS_LEN`, so it may
change when the keymap changes.

## Properties and security

- Read: permitted without additional characteristic-level security.
- Write with response: requires an encrypted BLE connection.
- Notify: enabled through the standard CCC descriptor.

An unencrypted write is rejected by the GATT server. Pair or bond the phone with Corney if the
BLE application reports insufficient encryption or authentication.

## Write validation

The server accepts only a write request (with response) at offset zero containing exactly four
bytes. Prepared writes, execute writes, and write commands without response are rejected. The
decoded value must be non-negative and less than `ZMK_KEYMAP_LAYERS_LEN`.

Rejected examples for the current keymap include:

- `FF FF FF FF`: signed value -1.
- `13 00 00 00`: layer 19, outside the current 0 through 18 range.
- `01`: too short.
- `01 00 00 00 00`: too long.
- Any otherwise valid payload written at a nonzero offset.
- Any payload sent as a prepared, execute, or write-without-response operation.

Rejected writes return an ATT error and do not schedule a ZMK layer change.

## Command and notification semantics

A successful write response confirms that the request was validated and queued. The layer change
runs later in Zephyr work context. `zmk_keymap_layer_to()` can internally pass through other
layers while deactivating the previous state. Notifications are briefly delayed and coalesced so
only the latest resulting layer is published, whether `layer_to()` came from BLE, a physical key,
or a macro.

After each executed request, the firmware sends one final notification containing the
authoritative highest active layer. This notification is sent even when the requested layer was
already active. If BLE transmit resources are temporarily busy, the latest value is retried.
Later physical layer behaviors remain authoritative and produce final-state notifications
through the same coalescing path.

If several writes arrive before deferred work starts, the latest accepted value occupies the
single pending slot. Clients that need one confirmation per command should wait for the final
notification before sending the next write.

## Phone BLE client test

1. Connect to and, when prompted, pair with `Corney`.
2. Discover the service and layer characteristic using the UUIDs above.
3. Enable notifications on the layer characteristic.
4. Read the characteristic and decode the four bytes as little-endian `int32`.
5. Use a write request (not write-without-response) to send a value such as
   `01 00 00 00`.
6. Confirm that one final notification arrives and that a subsequent read returns the same layer.
7. Exercise a non-default-to-non-default transition and confirm that no intermediate layer-zero
   notification appears.
8. Write the active layer again and confirm that the same value is notified once.
9. Try the rejected payload examples and confirm that the layer does not change.
10. Change the layer physically and confirm that the subscribed value follows the keyboard.
11. While the phone remains subscribed, run the `to_linux` macro and confirm that layer 1 is
    notified without an intermediate layer-0 notification.

Some phone applications do not expose nonzero-offset writes. That validation path can be checked
with any ATT client that supports explicit offsets.

## Build integration

- GitHub Actions builds discover the service through this repo's `zephyr/module.yml`, run the
  host/native integration checks, and retain enhanced, legacy-only, minimal-extension, peripheral,
  and settings-reset artifacts.
- `.gitlab-ci.yml` provides an equivalent alternative pipeline for a future GitLab mirror.
- Local `west build` commands should include `-DZMK_EXTRA_MODULES=$PWD` so the module sources
  are compiled alongside the Corney config.
- The feature can be disabled with `CONFIG_ZMK_GATT_LAYER_EXPOSITION=n`.
- The version 1 Keyboard Helper event extension is independently controlled by
  `CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION`; see `keyboard-helper-ble-v1.md`.
