# Corney website implementation check

The local website implements Home, Firmware, Build and Guide routes under `/corney/`. It uses the
two existing assembly photographs as sanitized WebP assets and keeps firmware downloads unpublished
until a versioned, physically accepted compatibility set is selected.

## Local checks

- `website/npm run check`: Astro diagnostics have 0 errors, 0 warnings and 0 hints; firmware data
  tests pass.
- `website/npm run build`: four static routes build; the post-build validator passes route, asset,
  base-path, topology, release-state, image-alt and WebP metadata checks.
- `website/npm audit --audit-level=low`: 0 vulnerabilities after upgrading to Astro 7 and using
  self-hosted fonts.
- `tests/run-host-tests.sh`: existing Corney host checks and release packaging tests pass.
- A local package smoke test using the five previously accepted UF2 files and their generated
  Kconfig outputs produced a checksummed ZIP with exactly five images plus `SHA256SUMS`.
- In a browser, all four routes had no horizontal overflow or broken images at 320 and 1440 CSS
  pixels. The first Tab focuses the visible skip link. No remote page resources loaded; primary
  content rendered from static HTML. The unpublished firmware page exposed no release asset links.
- Primary repo, Choc and MX source, keymap, contributor documents and Keyboard Helper URLs returned
  HTTP 200 during local verification.
- Representative text contrast ratios: muted copy 9.55:1, accent on page background 15.10:1,
  warning text 10.74:1 and button text 14.60:1.

## Public launch still pending

- Choose the first release version and accepted source revision.
- Create the tagged candidate, verify its published assets, and perform physical acceptance of both
  firmware topologies and settings reset.
- Update the checked-in release record and deploy Pages at the intended public URL. Then smoke-test
  the public site and add that URL to `readme.md`.

No public firmware release or Pages deployment was triggered by this implementation work.
