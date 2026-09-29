# Corney firmware release procedure

The existing five-target `build.yaml` matrix is the only release matrix. Ordinary branch and pull
request builds keep uploading the temporary `firmware` Actions artifact. A `vX.Y.Z` tag runs the same
host tests, native ZMK integration tests, generated topology checks and all five builds, then packages
one candidate release. Tag builds cannot request a custom Bluetooth name.

## Prepare a candidate

1. Select the accepted source commit and a unique `vX.Y.Z` tag. Record the revision before tagging.
2. Push the tag. The `Build ZMK firmware` workflow must pass all required jobs.
3. Its release job verifies the five UF2 files, their per-build provenance (tag, source revision,
   workflow run, Bluetooth name and SHA-256), then creates a prerelease candidate. It refuses an
   existing release tag and never overwrites release assets.
4. Download the individual UF2 files, `SHA256SUMS` and `corney-firmware-vX.Y.Z.zip`. Run
   `sha256sum -c SHA256SUMS` in the directory with all five files. Inspect the ZIP: it must contain
   exactly those five UF2 files and the same checksum manifest.

Only `corney-left-enhanced.uf2` and `corney-usb-dongle.uf2` are host-facing central images. Both
must have the default Bluetooth name `Corney`; the packaging step verifies their generated Kconfig
values. A manual custom-name build is not a standard public release.

## Accept and advertise

Test the candidate on physical hardware as one compatibility set:

- Direct Bluetooth: left central, right peripheral, pairing, keys and available pointing.
- USB Dongle: dedicated dongle and both halves, USB HID, display, both split links, independent
  reconnect, keys, available pointing and Keyboard Helper connection.
- Settings reset: confirm its destructive behavior and restore each operating image afterward.

After the candidate and checksums are accepted, update `website/src/data/firmware.js` with
`published: true`, the tag/version, full source revision, exact GitHub Release URL, default Bluetooth
name, physical acceptance flag, and all five SHA-256 values. Run `npm run check` and `npm run build`
in `website/`; inspect every rendered download. Promote the GitHub prerelease when ready. Until that
metadata update, the website intentionally shows no direct firmware downloads.

If a release is withdrawn, mark it non-current on GitHub and revert the website record to an earlier
accepted set or `published: false`. Preserve the old release assets and SHA evidence; publish a new
version for changed bytes. Website rollback does not alter installed firmware.
