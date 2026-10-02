import { copyFileSync, existsSync, mkdirSync, readdirSync, rmSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { modelManifest } from './models-manifest.mjs';

const websiteRoot = resolve(import.meta.dirname, '..');
const bodyRoot = resolve(websiteRoot, '..', 'body');
const outRoot = join(websiteRoot, 'public', 'models');

export function validateManifest(manifest = modelManifest, body = bodyRoot) {
  const errors = [];
  const targets = new Set();
  for (const { target, source } of manifest) {
    if (targets.has(target)) errors.push(`Duplicate model target: ${target}`);
    targets.add(target);
    if (!existsSync(join(body, source))) errors.push(`Missing model source for ${target}: body/${source}`);
  }
  return errors;
}

function listFiles(dir, prefix = '') {
  if (!existsSync(dir)) return [];
  return readdirSync(dir, { withFileTypes: true }).flatMap((entry) =>
    entry.isDirectory() ? listFiles(join(dir, entry.name), `${prefix}${entry.name}/`) : [`${prefix}${entry.name}`]);
}

export function syncModels(manifest = modelManifest, body = bodyRoot, out = outRoot) {
  const errors = validateManifest(manifest, body);
  const known = new Set(manifest.map(({ target }) => target));
  const stray = listFiles(out).filter((file) => file.endsWith('.stl') && !known.has(file));
  for (const file of stray) errors.push(`Unlisted STL in public/models/: ${file} (add it to the manifest, or delete public/models/ and rerun)`);
  if (errors.length) throw new Error(errors.join('\n'));
  rmSync(out, { recursive: true, force: true });
  for (const { target, source } of manifest) {
    mkdirSync(dirname(join(out, target)), { recursive: true });
    copyFileSync(join(body, source), join(out, target));
  }
  return manifest.length;
}

if (import.meta.url === `file://${process.argv[1]}`) {
  try {
    console.log(`Synced ${syncModels()} models into public/models/`);
  } catch (error) {
    console.error(`Model sync failed:\n${error.message}`);
    process.exit(1);
  }
}
