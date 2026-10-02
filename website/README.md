# Corney website

Static Astro site for `https://projects.maxistar.me/corney/`.

```sh
npm ci
npm run check
npm run build
npm run dev
```

`src/data/firmware.js` is the public firmware source of truth. The checked-in record stays
`published: false` until a versioned GitHub Release is physically accepted and its five UF2 hashes
have been verified. Source code changes and temporary CI artifacts do not activate download links.

The production build validates routes, base paths, topology mappings, release claims and image
metadata. Public WebP photographs are sanitized derivatives of the repository's `docs/` originals.

## 3D models

The STL files served on the Models page are not committed here. `scripts/models-manifest.mjs` lists
each published `models/<group>/<file>.stl` and the file under `../body/` it comes from, and
`scripts/sync-models.mjs` copies them into the git-ignored `public/models/`. The sync runs
automatically before `npm run dev`, `check` and `build`; run `npm run sync-models` to do it by hand.
It fails on a missing source, a duplicate target, or an unlisted STL in `public/models/`. To publish a
new model, add it to the manifest and to `src/pages/models.astro`; a test checks that both agree. The
website workflow also runs when an STL under `body/` changes.
