# Reduced Corney Firmware Matrix Verification

> Historical matrix baseline: the artifact names remain current, but the Studio-enabled central
> build documented here was superseded by `remove-zmk-studio-from-corney-firmware`.

Date: 2026-09-27

## Matrix acceptance

`build.yaml` declares exactly these release artifacts, in order:

1. `corney-left-enhanced`
2. `corney-right`
3. `settings-reset`

The host-side matrix assertion passed and confirmed that `corney-left-stock`,
`corney-left-legacy`, and `corney-left-extension-minimal` are absent. The alternative GitLab matrix
uses the same three artifact names.

## Automated tests

- `tests/run-host-tests.sh`: pass, including BLE contract tests, Cirque packet tests, metadata
  consistency, and the exact release-matrix assertion.
- `tests/run-zmk-integration-tests.sh`: pass for `combo-wrapper-ordinary`,
  `combo-wrapper-overlap`, `event-sources-both-halves`, and `input-split-pointing`.
- `tests/verify_cirque_build.py`: pass for the generated `corney_left`, `corney_right`, and
  `settings_reset` configurations and devicetrees.

## Firmware builds

All retained targets built successfully for `nice_nano_v2` from the arguments declared in
`build.yaml`. The local Studio build used the Python environment installed with `west`, which
contains the protobuf module required by nanopb generation; no firmware option was disabled as a
workaround.

Resolved configuration checks confirmed:

- `corney-left-enhanced`: ZMK Studio, Keyboard Helper extension, key events, combo events, layer
  events, diagnostics, the input-split proxy, and disconnect safety are enabled; the physical
  Cirque polling driver is absent.
- `corney-right`: input-split, I2C, and the Cirque polling driver are enabled; the physical sensor
  endpoint is present and the central proxy is absent.
- `settings-reset`: the nice!nano v2 board is selected and neither the Cirque topology nor the
  input-split proxy is present.

The accepted local artifact set is stored under
`.artifacts/simplify-corney-firmware-matrix/`:

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-left-enhanced.uf2` | 545280 bytes | `7c68559306e7bd9ba8d18fb30835d71fd7e19b2959c92d3bd4dbf02b796c0a44` |
| `corney-right.uf2` | 355840 bytes | `f7b7a74931fbe5ddcad91d0888dd13a3f5954c8e8d1eb1066bdf61b1280a6185` |
| `settings-reset.uf2` | 92672 bytes | `fd3072a56b359a56b08ad0c010b77ebe516846fc59ca21aa5c1059518a082d60` |

No retired UF2 filename is present in that directory.

## Physical verification decision

No additional physical regression run is required for this change. It removes matrix entries and
updates CI checks and documentation, but does not modify the build arguments, keymap, firmware
sources, BLE contract, or hardware topology of any retained artifact. The existing physical
acceptance for direct BLE operation and the universal right-side trackpad therefore remains the
applicable hardware evidence.
