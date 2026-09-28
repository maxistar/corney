# Corney Dongle Battery Levels Verification

Date: 2026-09-28

## Scope

The `corney-usb-dongle` custom status screen presents three independent power sources on the
existing 128x64 SSD1306:

```text
Half 1   78%
Half 2   64%
Dongle   42% USB
```

`Half 1` and `Half 2` are ZMK peripheral source slots, not permanent left/right identities. The
labels therefore remain valid if the halves bond in another order. A disconnected or not-yet-seen
slot is shown as `--`.

Only the dongle firmware changes. The direct central, both peripheral images, and settings-reset
remain byte-identical to the accepted `add-corney-dongle-oled` artifacts. The existing SSD1306
devicetree, orientation, idle blanking, and keyboard-activity wake configuration are unchanged.

## Data boundaries and limitations

- The dongle fetches Battery Service values from both split peripherals with
  `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING`.
- The auxiliary BAS proxy remains disabled. Battery values are display-local and do not add a
  Keyboard Helper capability or event.
- In pinned ZMK v0.3.0, a peripheral disconnect is reported with level zero. The display therefore
  treats remote `0%` as unavailable and shows `--`; a genuinely empty remote battery cannot be
  distinguished from a disconnected one.
- `Dongle` uses the local ZMK Battery Service state. `USB` means USB power is present and does not
  claim that a battery is charging. `BAT` means USB power is absent.
- Until a battery is installed on the dongle, accuracy of its battery-backed percentage remains a
  deferred physical check.

## Automated verification

- [x] Portable host tests cover initial unavailable slots, independent updates, bounds,
  disconnect-zero handling, local level changes, and USB/BAT formatting.
- [x] Native ZMK integration tests pass for combo delegation, both-half events, input-split
  pointing, and disconnect cleanup.
- [x] All five `nice_nano_v2` release targets build on ZMK commit
  `edf5c0814fd3ea202e43aad2d68fd32e882a518c` (v0.3.0 pin).
- [x] Generated configurations and devicetrees pass `tests/verify_cirque_build.py` for every target.
- [x] Only the dongle selects the custom screen and two-slot battery fetching. The built-in status
  screen and auxiliary BAS proxy are disabled, and the other four targets contain none of the new
  screen code.

## Resource change

| Dongle build | FLASH | RAM | UF2 size |
| --- | ---: | ---: | ---: |
| Accepted built-in OLED screen | 343,972 B | 68,452 B | 688,128 B |
| Custom battery screen | 328,564 B | 68,436 B | 657,408 B |
| Change | -15,408 B | -16 B | -30,720 B |

The custom build uses 40.51% of the 792 KiB application flash region and 26.11% of the 256 KiB
RAM region.

## Candidate artifacts

Candidate UF2 files and matching generated `.config` and `zephyr.dts` evidence are stored under
`.artifacts/show-corney-battery-levels-on-dongle/`.

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-left-enhanced.uf2` | 496128 bytes | `7aba0ccd18874ecdf3829aa3a69ed6db863d40a87026ecf3969bad7ef5ce696c` |
| `corney-left-peripheral.uf2` | 355840 bytes | `51069e75b64495508b1ecaff860dee4f62a85456fa7d382ec870c45f81f2396b` |
| `corney-right.uf2` | 355840 bytes | `f7b7a74931fbe5ddcad91d0888dd13a3f5954c8e8d1eb1066bdf61b1280a6185` |
| `corney-usb-dongle.uf2` | 657408 bytes | `675c8a22f8a7ce96112b784710e4675035262cc9e60d813c6a68d91fafdf674a` |
| `settings-reset.uf2` | 92672 bytes | `fd3072a56b359a56b08ad0c010b77ebe516846fc59ca21aa5c1059518a082d60` |

The four non-dongle hashes exactly match the accepted preceding OLED candidate. Only
`corney-usb-dongle` changed.

## Flash and rollback

For physical acceptance, flash only the new `corney-usb-dongle.uf2`. Existing half images and BLE
bonds should remain usable; no half reflash or re-pairing is expected.

If rollback is needed, flash
`.artifacts/add-corney-dongle-oled/corney-usb-dongle.uf2` on the dongle. The halves do not need to
be reflashed.

## Physical acceptance

- [x] Confirmed legible `Half 1`, `Half 2`, and `Dongle` values plus the USB marker.
- [x] Power-cycled each half independently and confirmed `--` then recovered level without breaking
  keys, pointing, USB HID, or disconnect cleanup.
- [x] Confirmed display idle blanking/wake and unchanged Keyboard Helper behavior.
- [x] Accepted dongle local data flow and USB indication; percentage accuracy remains pending until
  a dongle battery is installed.

Physical acceptance was user-confirmed on 2026-09-28. No connection or update-delay limitation was
reported. The pending dongle battery-backed percentage is an explicitly accepted hardware gap, not
a failure of the current USB-powered configuration.
