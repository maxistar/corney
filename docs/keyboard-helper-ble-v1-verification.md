# Keyboard Helper BLE v1 verification record

This record intentionally contains no captured typing content, addresses, credentials, or bonding
material.

## Reproducible build evidence

- Date: 2026-08-31
- ZMK revision: `v0.2.1` (`241ff39556b3685c9344c4c22fd9a655af8eb3ba`)
- Board: `nice_nano@2.0.0`
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

| Check | Expected observation | Status |
| --- | --- | --- |
| Matching halves | ordinary typing, split input, all combos, layers, reconnect, and legacy layer control remain correct | pass |
| Discovery | service `b34a0001-...`, capabilities `b34a0003-...`, and event `b34a0004-...` appear only on the central | pass on central; peripheral exclusion covered by build configuration |
| Security | capabilities read before pairing; unencrypted event CCC enable fails; encrypted enable succeeds | partial: encrypted enable pass |
| Stream start | first frame is the current layer with `STREAM_START | SNAPSHOT` | pass |
| Positions | key down/up from both halves use global positions `0..41` | partial: sampled positions pass; complete both-half sweep pending |
| Combos | all seven IDs and participant lists match layout metadata | pass |
| Battery | standard Battery Level `0x2a19` reads/notifies independently; capability bit 3 remains clear and no event type `0x04` appears | pass |
| Backpressure | fast input remains correct; any telemetry loss appears as sequence gaps and optionally diagnostic code 1 | pass for normal fast typing and automated saturation policy; no hardware saturation threshold measured |
| Ownership | first encrypted subscriber owns telemetry; a second receives busy; ownership releases on unsubscribe/disconnect | automated owner-state tests pass; multi-client hardware path unverified |
| Host coexistence | actual USB-host and BLE-host plus helper connection behavior is recorded | limitation: simultaneous HID-host and helper topology unverified |

The retained key, combo, layer, and stream-start evidence remains representative because those
frame formats and sources did not change in the standard-BAS-only rebuild.

Normal fast typing, layer switching through BLE, and all configured combos showed no visible input
latency. The hardware saturation threshold, unencrypted CCC rejection, complete 42-position sweep,
and simultaneous HID-host plus helper topology were not measured. Clients must therefore tolerate
connection-busy behavior and must not assume that a second BLE central can coexist with the active
HID host on this firmware/hardware combination.

Use nRF Connect or LightBlue and the umbrella contract's generic-phone procedure. Add only decoded
representative frames and pass/fail conclusions to this file after testing.
