# Corney website verification

# Website status

The public site is deployed at https://projects.maxistar.me/corney/ and the visual presentation was
accepted by the project owner. Home, Firmware, Build and Guide are available under `/corney/`. It uses the
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
- The project owner confirmed that the GitHub Actions tests pass after the host-test fix.
- A local package smoke test using the five previously accepted UF2 files and their generated
  Kconfig outputs produced a checksummed ZIP with exactly five images plus `SHA256SUMS`.
- In a browser, all four routes had no horizontal overflow or broken images at 320 and 1440 CSS
  pixels. The first Tab focuses the visible skip link. No remote page resources loaded; primary
  content rendered from static HTML. The unpublished firmware page exposed no release asset links.
- Primary repo, Choc and MX source, keymap, contributor documents and Keyboard Helper URLs returned
  HTTP 200 during local verification.
- Representative text contrast ratios: muted copy 9.55:1, accent on page background 15.10:1,
  warning text 10.74:1 and button text 14.60:1.
- The deployed Home, Firmware, Build and Guide routes were smoke-tested in a browser at 320 and
  1440 CSS pixels: no page-level horizontal overflow, broken images, missing image alternatives,
  or internal links escaping `/corney/` were found.

## Firmware release still pending

- Choose the first release version and accepted source revision.
- Create the tagged candidate, verify its published assets, and perform physical acceptance of both
  firmware topologies and settings reset.
- Update the checked-in release record only after the candidate is physically accepted.

The website deployment does not constitute acceptance or publication of a firmware release.
