# Corney Dongle Hybrid Status Screen Verification

Date: 2026-09-29

## Scope

The `corney-usb-dongle` custom compositor restores ZMK's standard output, local-battery, and
active-layer widgets at their built-in screen anchors and adds one identity-neutral remote battery
row below the local dongle indicator:

```text
┌────────────────────────┐
│ output          dongle │
│             78%   56%  │
│                        │
│ active layer           │
└────────────────────────┘
```

The two fields retain split-source-index order internally but carry no visible slot number or
left/right claim. Reversing slot assignment after a bond reset therefore does not change the user
meaning. Available values render as bounded `N%`; unavailable values render as `--`.

## Data and presentation boundaries

- ZMK's standard widgets exclusively own output/endpoint state, local dongle battery/USB state, and
  the highest active layer.
- The Corney listener subscribes only to `zmk_peripheral_battery_state_changed` and initializes from
  both retained split battery levels.
- The auxiliary BAS proxy remains disabled. Remote values remain display-local and do not change
  Keyboard Helper capabilities or the dongle's standard Battery Service meaning.
- Pinned ZMK v0.3.0 reports remote disconnect as level zero, so both disconnect and a genuine remote
  `0%` render as `--`.
- The standard local battery widget's lightning/power symbol indicates USB presence; it is not
  evidence that an optional dongle battery is installed or charging.
- The remote row uses Montserrat 12 in a 65-pixel right-aligned area at `y=20..34`. The unkerned
  pinned-font upper bound for `100% 100%` is 65 pixels. The stock top region uses Montserrat 16 with
  an 18-pixel line height, and the bottom layer region uses the 15-pixel small font.

## Automated verification

- [x] Portable host tests cover initial `-- --`, independent updates, bounded values,
  disconnect-zero handling, reversed slot arrival, and `100% 100%`.
- [x] `tests/verify_dongle_display_layout.py` derives glyph advances and line heights from the pinned
  Montserrat sources and verifies the remote row's horizontal and vertical bounds.
- [x] All four native ZMK integration fixtures pass: combo wrapper ordinary/overlap, both-half event
  sources, and input-split pointing/disconnect cleanup.
- [x] All five `nice_nano_v2` release targets build on ZMK commit
  `edf5c0814fd3ea202e43aad2d68fd32e882a518c` (v0.3.0 pin).
- [x] Generated configuration/devicetree verification passes for every target. Only the dongle
  selects the custom compositor, three standard widgets, Montserrat 12/16, and remote battery
  fetching; built-in status and auxiliary BAS proxy remain disabled.
- [x] The direct central, both peripherals, and settings-reset UF2 hashes exactly match the accepted
  preceding battery-screen release.

## Resource change

| Dongle build | FLASH | RAM | UF2 size |
| --- | ---: | ---: | ---: |
| Accepted compact battery screen | 328,564 B | 68,436 B | 657,408 B |
| Hybrid stock-geometry screen | 345,860 B | 68,612 B | 692,224 B |
| Change | +17,296 B | +176 B | +34,816 B |

The hybrid build uses 42.65% of the 792 KiB application flash region and 26.17% of the 256 KiB RAM
region. Relative to the earlier built-in OLED screen it adds 1,888 B FLASH, 160 B RAM, and 4,096 B
of UF2 output while also presenting both remote levels.

## Candidate artifacts

Candidate UF2 files and matching generated `.config` and `zephyr.dts` evidence are stored under
`.artifacts/refine-corney-dongle-status-screen/`.

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-left-enhanced.uf2` | 496128 bytes | `7aba0ccd18874ecdf3829aa3a69ed6db863d40a87026ecf3969bad7ef5ce696c` |
| `corney-left-peripheral.uf2` | 355840 bytes | `51069e75b64495508b1ecaff860dee4f62a85456fa7d382ec870c45f81f2396b` |
| `corney-right.uf2` | 355840 bytes | `f7b7a74931fbe5ddcad91d0888dd13a3f5954c8e8d1eb1066bdf61b1280a6185` |
| `corney-usb-dongle.uf2` | 692224 bytes | `0eee7cc882c368aa87ac47aa947d5561093a931f55f81ae276ecb1ac3e04c932` |
| `settings-reset.uf2` | 92672 bytes | `fd3072a56b359a56b08ad0c010b77ebe516846fc59ca21aa5c1059518a082d60` |

## Flash and rollback

Flash only
`.artifacts/refine-corney-dongle-status-screen/corney-usb-dongle.uf2`. Existing half images and BLE
bonds remain compatible and should not be replaced or reset.

If rollback is required, flash
`.artifacts/show-corney-battery-levels-on-dongle/corney-usb-dongle.uf2`. No half reflash is required.

## Physical acceptance

- [x] Confirm the output indicator at upper left, local battery at upper right, unlabeled remote pair
  beneath it, and active layer at lower left are legible and do not visibly overlap.
- [x] Change active layers and endpoint state and confirm the standard widgets update independently.
- [x] Power-cycle each half independently and confirm one field changes to `--` and later recovers
  while the other remains available.
- [x] Confirm idle blanking/wake, USB HID, keys from both halves, available pointing paths,
  disconnect cleanup, and Keyboard Helper retain their accepted behavior.
- [x] Rollback was not required because physical acceptance passed; the documented dongle-only
  rollback artifact remains available.

Physical acceptance was user-confirmed on 2026-09-29. The hybrid screen was reported to look good,
and no layout, input, reconnect, wake, or integration regression was reported.
