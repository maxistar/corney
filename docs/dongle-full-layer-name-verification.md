# Corney Dongle Full Layer Name Verification

Date: 2026-09-29

## Change

The dongle's custom status screen now reads the active layer's complete ZMK display name and draws
it beside a stationary keyboard symbol in the bottom row. The label uses all 112 pixels to the
right of the 14-pixel symbol and 2-pixel gap. LVGL circular scrolling is enabled on that label; text
that fits the row stays static, while longer names move through the available window at 20 pixels
per second. The configured LVGL default is 49 pixels per second, so the new rate is about 2.45
times slower (the nearest whole-pixel speed to exactly 2.5 times slower).

The existing output, local dongle battery, and two remote battery indicators retain their positions
and data paths. The implementation replaces only the dongle screen's stock layer widget, whose
14-byte formatting buffer truncated names before LVGL received them.

## Automated verification

- [x] Corney host tests pass, including source/layout checks for the ZMK active-layer API, layer
  event subscription, full-width name label, circular overflow mode, fixed keyboard symbol,
  reduced scroll speed, and dongle-only source selection.
- [x] Generated configuration and topology checks pass for all five Corney release targets.
- [x] Only `corney-usb-dongle` selects the custom screen; its stock layer widget is disabled. The
  other four retained release UF2 hashes are unchanged from the accepted battery-screen build.
- [x] The dongle target builds on pinned ZMK v0.3.0.

## Resource usage

| Dongle image | FLASH | RAM | UF2 size |
| --- | ---: | ---: | ---: |
| Accepted battery-screen image | 328,564 B | 68,436 B | 657,408 B |
| Full layer name candidate at 20 px/s | 345,800 B | 68,596 B | 691,712 B |
| Change | +17,236 B | +160 B | +34,304 B |

The candidate uses 42.64% of the 792 KiB application flash region and 26.17% of the 256 KiB RAM
region.

## Candidate artifact

The candidate and its generated configuration and devicetree are retained under
`.artifacts/show-full-layer-name-on-corney-dongle/`.

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-usb-dongle.uf2` | 691712 bytes | `37c69459b891b583ffd7e9dd11c3bde6a99d83de31c606c4ed522dafdb3a64a9` |

Rollback to the accepted battery-screen-only image at
`.artifacts/show-corney-battery-levels-on-dongle/corney-usb-dongle.uf2` (657408 bytes,
SHA-256 `675c8a22f8a7ce96112b784710e4675035262cc9e60d813c6a68d91fafdf674a`). Only the dongle needs
flashing; no half firmware or BLE bond change is expected.

## Physical acceptance

- [x] Confirmed a short name remains stationary and a long name, such as `MAC: Characters and
  Navigation Russian`, scrolls fully and legibly next to the keyboard symbol.
- [x] Confirmed the 20 px/s scroll speed is comfortable; layer changes, output and battery rows,
  display idle blanking/wake, and existing
  dongle typing, pointing, BLE reconnect, and Keyboard Helper behavior remain usable.

Physical acceptance was user-confirmed on 2026-09-29, including the reduced scrolling speed.
