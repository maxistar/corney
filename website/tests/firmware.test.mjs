import test from 'node:test';
import assert from 'node:assert/strict';
import { artifactNames, currentRelease, recovery, releaseView, topologies, validateFirmwareData } from '../src/data/firmware.js';

const syntheticRelease = () => ({
  published: true,
  physicallyAccepted: true,
  version: 'v1.2.3',
  sourceRevision: 'a'.repeat(40),
  bluetoothName: 'Corney',
  releaseUrl: 'https://github.com/maxistar/corney/releases/tag/v1.2.3',
  artifacts: artifactNames.map((name, index) => ({ name, sha256: index.toString(16).repeat(64) })),
});

test('the five artifact identities and both controller maps stay complete', () => {
  assert.deepEqual(artifactNames, [
    'corney-left-enhanced.uf2', 'corney-left-peripheral.uf2', 'corney-right.uf2',
    'corney-usb-dongle.uf2', 'settings-reset.uf2',
  ]);
  assert.deepEqual(topologies.map(({ id, controllers }) => [id, ...controllers.map(({ artifact }) => artifact)]), [
    ['direct', 'corney-left-enhanced.uf2', 'corney-right.uf2'],
    ['dongle', 'corney-usb-dongle.uf2', 'corney-left-peripheral.uf2', 'corney-right.uf2'],
  ]);
  assert.equal(recovery.artifact, 'settings-reset.uf2');
  assert.doesNotThrow(() => validateFirmwareData(currentRelease));
  assert.deepEqual(releaseView(), { published: false, label: 'No verified public firmware release yet.' });
});

test('accepted release yields immutable per-file and bundle URLs', () => {
  const release = syntheticRelease();
  const view = releaseView(release);
  assert.equal(view.artifacts['corney-usb-dongle.uf2'].url,
    'https://github.com/maxistar/corney/releases/download/v1.2.3/corney-usb-dongle.uf2');
  assert.equal(view.bundleUrl, 'https://github.com/maxistar/corney/releases/download/v1.2.3/corney-firmware-v1.2.3.zip');
  assert.equal(Object.keys(view.artifacts).length, 5);
});

test('publication rejects incomplete, mixed or unsafe release records', () => {
  const valid = syntheticRelease();
  const invalid = [
    { ...valid, artifacts: valid.artifacts.slice(0, 4) },
    { ...valid, artifacts: [...valid.artifacts.slice(0, 4), valid.artifacts[0]] },
    { ...valid, artifacts: valid.artifacts.map((a, i) => i === 2 ? { ...a, name: 'unknown.uf2' } : a) },
    { ...valid, artifacts: valid.artifacts.map((a, i) => i === 2 ? { ...a, sha256: 'bad' } : a) },
    { ...valid, bluetoothName: 'CorneyMX' },
    { ...valid, physicallyAccepted: false },
    { ...valid, releaseUrl: 'https://github.com/maxistar/corney/releases/tag/v1.2.4' },
    { ...valid, version: 'v1.2.4' },
    { published: false, version: 'v1.2.3' },
  ];
  for (const release of invalid) assert.throws(() => validateFirmwareData(release));
});

test('recovery cannot be used as an operating image', () => {
  const broken = topologies.map((topology) => ({ ...topology, controllers: topology.controllers.map((controller) => ({ ...controller })) }));
  broken[0].controllers[0].artifact = recovery.artifact;
  assert.throws(() => validateFirmwareData(currentRelease, broken));
});
