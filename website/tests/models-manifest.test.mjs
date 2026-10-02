import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdirSync, mkdtempSync, readFileSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { modelManifest } from '../scripts/models-manifest.mjs';
import { syncModels, validateManifest } from '../scripts/sync-models.mjs';

const pageModels = () => {
  const page = readFileSync(new URL('../src/pages/models.astro', import.meta.url), 'utf8');
  return [...page.matchAll(/model\(\s*'([^']+)'\s*,\s*'([^']+)'/g)].map(([, group, file]) => `${group}/${file}`);
};

test('manifest targets are unique and every source exists in body/', () => {
  assert.equal(new Set(modelManifest.map(({ target }) => target)).size, modelManifest.length);
  assert.deepEqual(validateManifest(), []);
});

test('models page and manifest list the same files', () => {
  assert.deepEqual([...pageModels()].sort(), modelManifest.map(({ target }) => target).sort());
});

test('sync reports a missing source, a duplicate target and a stray model', () => {
  const body = mkdtempSync(join(tmpdir(), 'body-'));
  const out = mkdtempSync(join(tmpdir(), 'out-'));
  writeFileSync(join(body, 'a.stl'), 'solid a');
  assert.throws(() => syncModels([{ target: 'g/a.stl', source: 'missing.stl' }], body, out), /Missing model source for g\/a\.stl/);
  assert.throws(() => syncModels([{ target: 'g/a.stl', source: 'a.stl' }, { target: 'g/a.stl', source: 'a.stl' }], body, out), /Duplicate model target/);
  mkdirSync(join(out, 'g'));
  writeFileSync(join(out, 'g', 'stray.stl'), 'solid stray');
  assert.throws(() => syncModels([{ target: 'g/a.stl', source: 'a.stl' }], body, out), /Unlisted STL in public\/models\/: g\/stray\.stl/);
});

test('sync copies sources byte for byte and drops removed entries', () => {
  const body = mkdtempSync(join(tmpdir(), 'body-'));
  const out = mkdtempSync(join(tmpdir(), 'out-'));
  writeFileSync(join(body, 'a.stl'), 'solid a');
  assert.equal(syncModels([{ target: 'g/a.stl', source: 'a.stl' }], body, out), 1);
  assert.equal(readFileSync(join(out, 'g', 'a.stl'), 'utf8'), 'solid a');
});
