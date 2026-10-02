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

## Prototype dongle case

The Models page has a "USB dongle case (prototype)" set with three STL files copied from
`body/dongle/` (source last changed in `e1ed106`, byte-identical at copy time): bottom shell, upper
shell and reset button. The Build page has a prototype block (30 x 70 mm breadboard design basis,
adjustable OpenSCAD parameters, screen cut-out still being refined) and the Guide dongle section links
to it as optional. The Home page is unchanged.

- `website/npm run check` and `npm run build`: 0 errors, 4 data tests pass, the post-build validator
  passes with five routes.
- Local preview: the dongle set loads its STL (HTTP 200), the viewer renders the shell with no console
  errors, and the Build page has no horizontal overflow at 320 CSS pixels.

## Models built from sources

The 3D models are no longer committed under `website/public/models/`. `website/scripts/models-manifest.mjs`
maps each published model to its file in `body/`, and `scripts/sync-models.mjs` copies them before
`dev`, `check` and `build`; the output directory is git-ignored. The website workflow also runs when an
STL under `body/` changes.

- Before untracking, the generated directory was byte-identical to the 16 committed copies and the 3
  untracked dongle copies.
- From an empty `public/models/`, `npm run check` (8 tests pass) and `npm run build` pass, and the
  resulting `dist/` is identical to the previous build output, including all 19 STL files.
- Fault injection: an unlisted STL, a missing source (the build aborts before Astro runs) and a
  duplicate target each fail with a message naming the entry; the mapping was restored afterwards.

## Firmware release still pending

- Choose the first release version and accepted source revision.
- Create the tagged candidate, verify its published assets, and perform physical acceptance of both
  firmware topologies and settings reset.
- Update the checked-in release record only after the candidate is physically accepted.

The website deployment does not constitute acceptance or publication of a firmware release.
