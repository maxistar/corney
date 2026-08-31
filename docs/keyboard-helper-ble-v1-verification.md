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

| Build | Extension configuration | FLASH | RAM | Result |
| --- | --- | ---: | ---: | --- |
| Central legacy | extension disabled | 237,656 B | 59,660 B | pass |
| Central enhanced | all available capabilities | 240,780 B | 60,284 B | pass |
| Central minimal | extension enabled, optional event sources disabled | 239,436 B | 60,276 B | pass |
| Peripheral/right | no host-facing extension | 172,616 B | 33,884 B | pass |
| Settings reset | utility image | 46,188 B | 11,552 B | pass |

Enhanced versus legacy central delta is 3,124 B FLASH and 624 B RAM. The enhanced image uses
29.69% of 792 KiB FLASH and 23.00% of 256 KiB RAM, including the fixed 16-entry queue.

## Hardware verification

Do not treat the extension as hardware-verified until every row below has evidence.

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

| Check | Expected observation | Status |
| --- | --- | --- |
| Matching halves | ordinary typing, split input, all combos, layers, reconnect, and legacy layer control remain correct | pending |
| Discovery | service `b34a0001-...`, capabilities `b34a0003-...`, and event `b34a0004-...` appear only on the central | partial: central pass; peripheral observation pending |
| Security | capabilities read before pairing; unencrypted event CCC enable fails; encrypted enable succeeds | partial: encrypted enable pass |
| Stream start | first frame is the current layer with `STREAM_START | SNAPSHOT` | pass |
| Positions | key down/up from both halves use global positions `0..41` | partial: sampled positions pass; complete both-half sweep pending |
| Combos | all seven IDs and participant lists match layout metadata | pass |
| Battery | central battery frames appear when available; no right battery is fabricated | pending |
| Backpressure | fast input remains correct; any telemetry loss appears as sequence gaps and optionally diagnostic code 1 | pending |
| Ownership | first encrypted subscriber owns telemetry; a second receives busy; ownership releases on unsubscribe/disconnect | pending |
| Host coexistence | actual USB-host and BLE-host plus helper connection behavior is recorded | pending |

Use nRF Connect or LightBlue and the umbrella contract's generic-phone procedure. Add only decoded
representative frames and pass/fail conclusions to this file after testing.
