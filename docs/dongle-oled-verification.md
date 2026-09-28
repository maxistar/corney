# Corney USB Dongle OLED Verification

Date: 2026-09-28

## Scope

`corney-usb-dongle` now selects one four-wire 128x64 SSD1306 at I2C address `0x3c` and renders
ZMK's built-in status screen. The display is isolated to the USB-powered dongle. The direct central,
left peripheral, right peripheral, and settings-reset targets remain display-free; their accepted
matrix, optional Cirque, split, pointing, and Keyboard Helper roles do not change.

The configuration was adapted from the hardware-tested `my-zmk-test` overlay. Corney's pinned
Zephyr binding supports the geometry, orientation, precharge, and 100 ms ready delay used here but
does not support the newer `use-internal-iref` property, so that property is intentionally absent.

## Wiring

Use a 3.3 V-compatible four-pin I2C SSD1306 module:

| SSD1306 pin | Dongle nice!nano v2 / Pro Micro pin |
| --- | --- |
| `VCC` | `3V3` |
| `GND` | `GND` |
| `SDA` | `D2` / `P0.17` |
| `SCL` | `D3` / `P0.20` |

The expected address is `0x3c`. Do not use `RAW` or 5 V unless the exact module has separately been
verified to accept that supply. Neither keyboard half is rewired for this change.

## Automated verification

- [x] Portable host tests pass, including the exact five-artifact release matrix.
- [x] Native ZMK integration tests pass for combo delegation, both-half events, and input-split
  pointing/disconnect recovery.
- [x] All five `nice_nano_v2` release targets build on pinned ZMK v0.3.0.
- [x] Generated configurations and devicetrees pass `tests/verify_cirque_build.py` for every target.
- [x] The dongle resolves I2C, SSD1306, Zephyr/ZMK display support, built-in status screen, and
  one-bit LVGL output while retaining USB, BLE split central, both pointing proxies, disconnect
  safety, and the complete Keyboard Helper extension.
- [x] The dongle generated devicetree selects exactly one 128x64 SSD1306 at `0x3c`, contains no
  physical Cirque or GPIO matrix, and the other four targets contain no dongle display.

## Resource change

Linker memory usage and UF2 size are compared with the accepted display-free dongle whose UF2 hash
is `d8577d60330acd78af3eca2b65f170738285cfd2bf35ec7b99f1e02ebd1144f0`.

| Dongle build | FLASH | RAM | UF2 size |
| --- | ---: | ---: | ---: |
| Display-free baseline | 242,704 B | 62,132 B | 485,888 B |
| Built-in OLED status screen | 343,972 B | 68,452 B | 688,128 B |
| Change | +101,268 B | +6,320 B | +202,240 B |

The display-enabled build uses 42.41% of the 792 KiB application flash region and 26.11% of the
256 KiB RAM region.

## Candidate artifacts

Candidate UF2 files and matching generated `.config` and `zephyr.dts` evidence are stored under
`.artifacts/add-corney-dongle-oled/`.

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-left-enhanced.uf2` | 496128 bytes | `7aba0ccd18874ecdf3829aa3a69ed6db863d40a87026ecf3969bad7ef5ce696c` |
| `corney-left-peripheral.uf2` | 355840 bytes | `51069e75b64495508b1ecaff860dee4f62a85456fa7d382ec870c45f81f2396b` |
| `corney-right.uf2` | 355840 bytes | `f7b7a74931fbe5ddcad91d0888dd13a3f5954c8e8d1eb1066bdf61b1280a6185` |
| `corney-usb-dongle.uf2` | 688128 bytes | `c5e6b8606ff9a921da79c93d1a68e95cdd628b0ed9faf7e98dc5f0cf0ee877ad` |
| `settings-reset.uf2` | 92672 bytes | `fd3072a56b359a56b08ad0c010b77ebe516846fc59ca21aa5c1059518a082d60` |

The four non-dongle artifacts are byte-identical to the accepted preceding dongle-topology
candidate. Only `corney-usb-dongle` changed.

## Physical acceptance

- [x] Wired the compatible SSD1306 and flashed only the new `corney-usb-dongle` candidate.
- [x] Confirmed legible built-in ZMK status content in the expected orientation after boot.
- [x] Confirmed idle blanking and wake after keyboard activity.
- [x] Confirmed USB enumeration, typing from both halves, available pointing paths, Keyboard Helper,
  and independent half reconnect while the display is active.
- [x] The candidate passed, so rollback was not required. The preceding display-free dongle UF2
  remains available without reflashing either half.

Physical acceptance was user-confirmed on 2026-09-28: all requested manual tests passed and the
display looked as expected. The display changes only the central image; neither peripheral was
reflashed for this check.
