# Keyboard Helper BLE v1 verification record

This record intentionally contains no captured typing content, addresses, credentials, or bonding
material.

## Reproducible build evidence

- Date: 2026-08-31
- ZMK revision: `v0.2.1` (`241ff39556b3685c9344c4c22fd9a655af8eb3ba`)
- Original requested board string: `nice_nano@2.0.0` (later found to resolve to the v1 board
  definition on this Zephyr 3.5 baseline; superseded by the corrected v2 artifacts below)
- Protocol: `1.0`
- Host tests: passed
- ZMK native event/combo integration tests: passed
- Position metadata: 42 unique global positions
- Combo metadata: IDs `1..7`, complete and consistent between layout and devicetree

### Hosted CI status

The first GitHub Actions native-test run failed before executing a test because the unanchored
`.gitignore` entry `zmk` also excluded `tests/zmk` from the repository. The rule is now anchored to
the root checkout as `/zmk`, and the integration fixtures are included. The next run reached the
fixtures but invoked `west build` outside the isolated west workspace, so all cases failed before
compilation. The workflow now runs the checkout's test script with `$base_dir` as its working
directory and enables verbose build output. All three cases pass locally in the same
`zmkfirmware/zmk-build-arm:stable` container. The subsequent hosted GitHub Actions rerun completed
successfully, including host tests, native integration tests, and the firmware build matrix.

| Build | Extension configuration | FLASH | RAM | Result |
| --- | --- | ---: | ---: | --- |
| Central legacy | extension disabled | 237,656 B | 59,660 B | pass |
| Central enhanced | release configuration, including ZMK Studio RPC | 268,904 B | 87,538 B | pass |
| Central extension-only | all extension capabilities, ZMK Studio disabled | 240,512 B | 60,276 B | pass |
| Central minimal | extension enabled, optional event sources disabled | 239,392 B | 60,276 B | pass |
| Peripheral/right | no host-facing extension | 172,616 B | 33,884 B | pass |
| Settings reset | utility image | 46,188 B | 11,552 B | pass |

Extension-only versus legacy central delta is 2,856 B FLASH and 616 B RAM. The release-enhanced
image also includes ZMK Studio RPC, so its 31,248 B FLASH and 27,878 B RAM delta is not attributable
to the BLE extension alone. All keyboard variants retain ZMK's standard Battery Service; capability
bit 3 is masked clear, custom event type `0x04` is reserved, and split-central peripheral battery
fetching remains disabled.

### Concurrent-transport platform evidence (2026-09-01)

- The supported central configuration explicitly sets `CONFIG_BT_MAX_CONN=6` and
  `CONFIG_BT_MAX_PAIRED=6`: five ZMK host profiles plus the right split peripheral. The transport
  has a build assertion requiring `CONFIG_BT_MAX_CONN` to cover configured split-central
  peripherals plus at least two host subscribers.
- ZMK `v0.2.1` pins Zephyr `v3.5.0+zmk-fixes`. In that revision, targeted
  `bt_gatt_notify(conn, ...)` checks the selected connection's CCC state and copies notification
  data into a Bluetooth buffer before returning. The worker's per-attempt stack frame therefore
  remains valid without a completion callback; a successful return means the caller-owned bytes
  have already been copied, not that the radio transmission has completed.
- A disconnect callback removes only the matching registry slot and releases its retained
  `bt_conn` reference. Managed CCC restoration after encryption is idempotent, so restoring one
  persisted subscription does not reset another subscriber.
- The retained event history remains 16 entries and stores each encoded live frame once. The
  subscriber registry is fixed at build time from `CONFIG_BT_MAX_CONN`; there is no heap allocation
  and no single-owner compatibility branch.

The concurrent transport was then rebuilt in the same official ZMK container and pinned workspace:

| Post-change build | FLASH | RAM | Delta from pre-change counterpart | Result |
| --- | ---: | ---: | ---: | --- |
| Central enhanced | 270,264 B | 87,906 B | +1,360 B FLASH, +368 B RAM | pass |
| Peripheral/right | 172,616 B | 33,884 B | unchanged | pass |

The resolved central `.config` contains `CONFIG_BT_MAX_CONN=6`,
`CONFIG_BT_MAX_PAIRED=6`, `CONFIG_ZMK_SPLIT_BLE_CENTRAL_PERIPHERALS=1`, and a 16-entry retained
history. The central uses 33.32% of its 792 KiB application FLASH region and 33.53% of its 256 KiB
RAM region. The pinned native ZMK event/combo integration suite also passes with the concurrent
sources.

## ZMK v0.3.0 upgrade evidence (2026-09-05)

- Manifest revision: `v0.3.0` (`edf5c0814fd3ea202e43aad2d68fd32e882a518c`)
- Imported Zephyr revision: `v3.5.0+zmk-fixes` (`dacab4875df72109b96cc8977547a0dc04875bcd`)
- Board: `nice_nano_v2` (legacy Zephyr 3.5 identifier, not HWMv2 syntax)
- Protocol: unchanged at `1.0`
- Formatting and host tests: passed
- ZMK native event/combo integration tests: all three cases passed
- Firmware source adaptations: none required

All targets declared in `build.yaml` were built from the updated west workspace with Zephyr SDK
0.16.8. The Studio-enabled target additionally used an isolated Python 3.12 environment containing
`protobuf`, `grpcio-tools`, and `setuptools<81`; the setuptools upper bound preserves the
`pkg_resources` API used by the pinned nanopb generator.

| Build | Extension configuration | FLASH | RAM | UF2 size | Result |
| --- | --- | ---: | ---: | ---: | --- |
| Central legacy | extension disabled | 238,568 B | 58,620 B | 477,184 B | pass |
| Central enhanced | release configuration, including ZMK Studio RPC | 271,296 B | 86,874 B | 542,720 B | pass |
| Central minimal | extension enabled, optional event sources disabled | 241,112 B | 59,604 B | 482,304 B | pass |
| Peripheral/right | no host-facing extension | 173,320 B | 33,900 B | 347,136 B | pass |
| Settings reset | utility image | 46,188 B | 11,552 B | 92,672 B | pass |

Every corrected artifact resolves `CONFIG_BOARD_NICE_NANO_V2=y`; all normal keyboard images also
resolve `CONFIG_ZMK_BATTERY_NRF_VDDH=y`. The enhanced central resolves
`CONFIG_BT_MAX_CONN=6`, `CONFIG_BT_MAX_PAIRED=6`,
`CONFIG_ZMK_SPLIT_BLE_CENTRAL_PERIPHERALS=1`, `CONFIG_ZMK_STUDIO=y`, and a 16-entry Keyboard
Helper history. This retains five host-profile slots alongside the split peripheral and satisfies
the build assertion reserving capacity for at least two concurrent host subscribers. The
`v0.3.0` update did not change BLE UUIDs, capability bits, frame layouts, keymap metadata, or the
legacy layer-control contract.

### v0.3.0 hardware acceptance status

The initial `v0.3.0` images were flashed to both halves and ordinary keyboard behavior was reported
to work as expected, but BLE Battery Level read 0%. Inspection showed those images had resolved
`nice_nano@2.0.0` to the pin-compatible v1 board definition and selected its absent ADC voltage
divider. The previous 100% reading was also not valid: ZMK `v0.2.1` left the Battery Service's
default value unchanged when the sampled state remained zero, while `v0.3.0` synchronizes zero to
the service. These preliminary images are superseded and are not accepted artifacts.

The corrected `nice_nano_v2` images select the controller's VDDH sensor and passed the full
automated matrix. Matching corrected images were then flashed to both halves and manually accepted:

- ordinary typing, both-half input, split reconnect, configured combos, and layers worked as
  expected;
- pointing movement and scrolling worked over USB and BLE, including profile switching;
- ZMK Studio connected and exposed the keymap;
- an encrypted generic BLE client successfully exercised legacy layer read/write/notify, Keyboard
  Helper discovery and capabilities, stream-start snapshot, representative key/combo/layer events,
  disconnect, and resubscription;
- existing bonds and settings survived the update, so the settings-reset image was not required;
- standard BLE Battery Level reported a plausible nonzero value independently of Keyboard Helper
  telemetry.

This confirms that selecting the v2 VDDH sensor fixes the invalid zero reading and that the accepted
ZMK `v0.3.0` images preserve the required hardware behavior.

## Cirque polling build evidence (2026-09-06)

The left/central shield now contains the I2C-only Cirque Pinnacle polling driver. The sensor is at
address `0x2a`, has no DR connection, and is polled every 8 ms while the central is active. Host
tests cover status filtering, malformed packet rejection, signed relative-axis boundaries, wheel
decoding, and primary-button press/hold/release transitions. The existing BLE metadata/transport
host tests and all three native ZMK event/combo integration tests also pass.

Every target declared in `build.yaml` was rebuilt from the pinned ZMK `v0.3.0` workspace:

| Build | FLASH | RAM | UF2 size | Result |
| --- | ---: | ---: | ---: | --- |
| Central legacy | 242,576 B | 58,836 B | 485,376 B | pass |
| Central enhanced with Studio | 275,652 B | 87,162 B | 551,424 B | pass |
| Central minimal | 245,120 B | 59,820 B | 490,496 B | pass |
| Peripheral/right | 173,320 B | 33,900 B | 347,136 B | pass |
| Settings reset | 46,188 B | 11,552 B | 92,672 B | pass |

Generated configuration and devicetree output contain `CONFIG_I2C=y`,
`CONFIG_INPUT_CORNEY_PINNACLE_POLLING=y`, and `glidepoint@2a` only for central targets. The right
and settings-reset artifacts contain none of them. Hardware movement, tap, suspend/resume, and
battery-impact checks remain pending until matching images are flashed.

### Rollback

Until hardware acceptance is complete, retain `v0.2.1` as the rollback revision. To roll back,
restore `revision: v0.2.1` in `config/west.yml`, run `west update`, rebuild matching images for both
halves, and flash both halves. Use the settings-reset image only if normal rollback and re-pairing
do not restore connectivity.

## Hardware verification

The table distinguishes hardware observations from automated coverage and explicitly retained
limitations. An unverified topology is not implied to be supported.

### Observed on hardware (2026-08-31)

- The enhanced central image was built with `CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y` and flashed.
- A generic BLE scanner discovered service `b34a0001-...` with the legacy layer,
  capabilities `b34a0003-...`, and event `b34a0004-...` characteristics.
- Enabling event notifications succeeded on the paired connection.
- Sampled key press/release notifications used the reviewed `KEY` frame layout; action changed
  between down/up while the physical global position remained stable and matched the layout.
- All seven firmware-resolved combos produced the expected `COMBO` notifications with matching
  stable IDs and participant positions without changing their normal keyboard behavior.
- A fresh subscription delivered the current-layer frame first, with protocol `1`, event type
  `LAYER`, flags `STREAM_START | SNAPSHOT` (`0x03`), payload length `4`, sequence `0`, restore
  cause, and unknown origin (`0xff`).
- The corrected standard-BAS-only image was flashed and manually verified. Battery Level `0x2a19`
  remained available independently of the extension, reserved capability bit 3 remained clear,
  and no reserved event type `0x04` appeared during the observation interval.

### Concurrent hardware observation (2026-09-01)

- Manual verification after flashing the concurrent transport confirmed that multiple clients can
  subscribe to and receive enhanced event streams at the same time. The former ATT `0x11`
  single-owner rejection no longer occurs while connection capacity remains available.
- The two clients observed matching live frame order and sequence values after their independent
  stream starts. Reconnect/disable of one client did not restart or interrupt the other.
- Concurrent phone/desktop use and actual capacity exhaustion were also verified successfully.
  Deliberately starving one subscriber remains an accepted non-blocking follow-up.

| Check | Expected observation | Status |
| --- | --- | --- |
| Matching halves | ordinary typing, split input, all combos, layers, reconnect, and legacy layer control remain correct | pass |
| Discovery | service `b34a0001-...`, capabilities `b34a0003-...`, and event `b34a0004-...` appear only on the central | pass on central; peripheral exclusion covered by build configuration |
| Security | capabilities read before pairing; unencrypted event CCC enable fails; encrypted enable succeeds | partial: encrypted enable pass |
| Stream start | first frame is the current layer with `STREAM_START | SNAPSHOT` | pass |
| Positions | key down/up from both halves use global positions `0..41` | partial: sampled positions pass; complete both-half sweep pending |
| Combos | all seven IDs and participant lists match layout metadata | pass |
| Battery | standard Battery Level `0x2a19` reads/notifies independently; capability bit 3 remains clear and no event type `0x04` appears | pass |
| Backpressure | fast input remains correct; any telemetry loss appears as sequence gaps and optionally diagnostic code 1 | pass for normal fast typing and automated saturation policy; deliberate slow-client hardware observation pending as a non-blocking follow-up |
| Concurrent subscribers | two encrypted subscribers each receive an initial snapshot and matching live order/sequence; either can disconnect without restarting the other | pass |
| Capacity | subscriber rejection occurs only when configured connection resources are exhausted and leaves existing subscribers active | pass |
| Host coexistence | actual USB-host and BLE-host plus helper connection behavior is recorded | partial: concurrent phone/desktop BLE use passes; simultaneous USB HID host plus helper topology remains unverified |

The retained key, combo, layer, and stream-start evidence remains representative because those
frame formats and sources did not change in the standard-BAS-only rebuild.

Normal fast typing, layer switching through BLE, and all configured combos showed no visible input
latency. Deliberate slow-client behavior, unencrypted CCC rejection, the complete 42-position
sweep, and simultaneous USB HID-host plus helper topology were not measured. Clients must tolerate
connection-resource exhaustion and per-subscriber sequence gaps without disrupting HID behavior
or other subscribers.

Use nRF Connect or LightBlue and the umbrella contract's generic-phone procedure. Add only decoded
representative frames and pass/fail conclusions to this file after testing.
