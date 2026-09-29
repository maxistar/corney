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
