# Keyboard Helper BLE extension v1

Corney's central (`corney_left`) firmware implements version 1.0 of the Keyboard Helper BLE
contract documented by the umbrella repository in `docs/zmk-ble-keyboard-contract-v1.md`. The
right/peripheral firmware preserves ordinary ZMK split position forwarding and never advertises
the host-facing service.

## Build modes

`CONFIG_ZMK_GATT_LAYER_EXPOSITION=y` keeps the legacy four-byte layer register available.
`CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y` adds the capabilities and encrypted event stream. The
extension defaults off in Kconfig so legacy behavior remains an explicit rollback path; the
enhanced release matrix enables it explicitly.

Optional capability switches are:

- `CONFIG_ZMK_KEYBOARD_HELPER_KEY_EVENTS`
- `CONFIG_ZMK_KEYBOARD_HELPER_COMBO_EVENTS`
- `CONFIG_ZMK_KEYBOARD_HELPER_LAYER_EVENTS`
- `CONFIG_ZMK_KEYBOARD_HELPER_DIAGNOSTICS`

The capabilities value is derived from these compiled options. Disabling one source clears only
its bit. Capability bit 3 and event type `0x04` are reserved and never emitted. Standard ZMK
Battery Service is the sole battery source in stock and enhanced modes. Peripheral battery fetching
remains disabled and right-half battery support is deferred.

## Event capture

- Physical key events subscribe directly to central-observed `zmk_position_state_changed` and
  retain global positions `0..41` from both halves.
- Seven combo behaviors use a Corney wrapper that publishes stable combo ID and participants,
  then invokes the original ZMK binding unchanged.
- Legacy and enhanced layer reporting share the same authoritative cached highest layer.
- The custom stream contains no HID usage, keycode, Unicode, resolved behavior, or text.

`layout_corney.json` and `corney_macros.dtsi` carry the same combo IDs and positions. Run
`tests/run-host-tests.sh` to validate the mapping and the exact version 1 frame examples. Run
`tests/run-zmk-integration-tests.sh` to verify raw combo-participating positions, wrapper
press/release delegation, overlap resolution, non-text behavior, and absence of duplicate resolved
events in ZMK's native test runner.

## Queue and loss behavior

Event producers allocate one global sequence and perform a bounded, non-blocking append into
16-entry retained history. Each subscriber has its own cursor and retry state while notification
runs later in work context. A transient `-ENOMEM` or `-EAGAIN` result is retried no more than three
times, 10 ms apart, for the affected subscriber without delaying other ready subscribers.
Permanent failures, an exhausted retry budget, and history overwritten before a slow subscriber
consumes it produce a gap only in that subscriber's observed stream. Shared retention overflow
also schedules diagnostic code 1 when capacity becomes available.

## Concurrent subscribers and security

Capabilities remain readable before encryption. Enabling the event CCC requires an encrypted
connection. Every eligible connection receives an independent subscriber slot while configured
Bluetooth capacity remains available. Corney configures six simultaneous connections: one for
the right split peripheral and five host profiles, so at least two encrypted event subscribers
can coexist. `BT_ATT_ERR_INSUFFICIENT_RESOURCES` is reserved for actual subscriber/connection
capacity exhaustion and never transfers or cancels an existing subscription.

<<<<<<< HEAD
Telemetry ownership is independent of the active ZMK HID profile. Whether the controller keeps a
phone helper and a normal BLE host connected simultaneously was not verified on hardware and is not
a version 1 product guarantee. Clients must handle connection failure or busy state without
disrupting normal HID behavior. Every new owner starts a fresh sequence epoch and receives an
authoritative layer snapshot marked `STREAM_START | SNAPSHOT` before live events.
=======
Telemetry subscription is independent of the active ZMK HID profile. Every subscriber receives a
targeted authoritative layer snapshot marked `STREAM_START | SNAPSHOT` before its future live
events. A subscriber joining, leaving, slowing down, or reconnecting does not reset or purge any
other subscriber's stream.

Firmware flashed before concurrent subscriber support is a development-only build and must be
reflashed. Protocol 1.0 UUIDs, capability bytes, and event frame layouts are unchanged; there is
no single-owner compatibility mode or version branch.
>>>>>>> ccedd64 (allow several streams, add 3d model)

## Local verification

Run the portable tests:

```sh
tests/run-host-tests.sh
```

Build the enhanced central:

```sh
west build -p always -s zmk/app -d build/enhanced-left -b nice_nano@2.0.0 -- \
  -DZMK_CONFIG="$PWD/config" \
  -DZMK_EXTRA_MODULES="$PWD" \
  -DSHIELD=corney_left \
  -DCONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y
```

Build `corney_right` without the extension override. A GATT database change can require clearing
old bonds or flashing the `settings_reset` image before pairing again.
