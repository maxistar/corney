# ZMK Studio Removal Verification

Date: 2026-09-27

## Release configuration

The supported `corney-left-enhanced` build explicitly sets `CONFIG_ZMK_STUDIO=n` and no longer
uses the `studio-rpc-usb-uart` snippet. Its resolved configuration omits ZMK Studio, Studio RPC,
UART and BLE Studio transports, USB CDC ACM, nanopb, and dynamic keymap settings storage.

The same configuration retains USB and BLE HID, the input-split proxy, and the complete Keyboard
Helper capability set: key events, combo events, layer events, and diagnostics. The alternative
GitLab matrix uses the same central build arguments.

## Automated verification

- `tests/run-host-tests.sh`: pass, including the Studio-disabled release-matrix assertion.
- `tests/run-zmk-integration-tests.sh`: pass for all four native ZMK fixtures.
- `tests/verify_cirque_build.py`: pass for `corney_left`, `corney_right`, and `settings_reset`.
- All three `nice_nano_v2` release entries build successfully from the declared arguments.

## Resource change

Relative to the accepted Studio-enabled central build recorded by the preceding matrix change:

| Central build | FLASH | RAM | UF2 size |
| --- | ---: | ---: | ---: |
| Studio enabled | 272,452 B | 87,146 B | 545,280 B |
| Studio disabled | 243,864 B | 59,836 B | 487,936 B |
| Change | -28,588 B | -27,310 B | -57,344 B |

## Accepted build artifacts

The candidate set is stored under `.artifacts/remove-zmk-studio-from-corney-firmware/`:

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-left-enhanced.uf2` | 487936 bytes | `22ab0ea130ab4506743f012c8ec0f7d3582f0e750c2d0153823662e201be8a33` |
| `corney-right.uf2` | 355840 bytes | `f7b7a74931fbe5ddcad91d0888dd13a3f5954c8e8d1eb1066bdf61b1280a6185` |
| `settings-reset.uf2` | 92672 bytes | `fd3072a56b359a56b08ad0c010b77ebe516846fc59ca21aa5c1059518a082d60` |

The right and settings-reset binaries are byte-identical to the accepted artifacts from the
preceding matrix simplification; only the central changed.

## Physical acceptance

User-confirmed on physical hardware on 2026-09-27 with the Studio-free central and matching
universal right image:

- ordinary typing works over USB and BLE;
- input from both halves and split reconnect work as expected;
- right-side pointing and scrolling work as expected;
- Keyboard Helper discovery and events work as expected;
- legacy layer control works as expected.

No retained behavior regressed and no additional limitation was reported for this change.
