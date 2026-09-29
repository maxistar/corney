export const artifactNames = Object.freeze([
  'corney-left-enhanced.uf2',
  'corney-left-peripheral.uf2',
  'corney-right.uf2',
  'corney-usb-dongle.uf2',
  'settings-reset.uf2',
]);

export const topologies = Object.freeze([
  Object.freeze({
    id: 'direct',
    name: 'Direct Bluetooth',
    summary: 'The left half is the central. Your computer connects to the keyboard over Bluetooth.',
    controllers: Object.freeze([
      Object.freeze({ role: 'Left half · central', artifact: 'corney-left-enhanced.uf2' }),
      Object.freeze({ role: 'Right half · peripheral', artifact: 'corney-right.uf2' }),
    ]),
  }),
  Object.freeze({
    id: 'dongle',
    name: 'USB Dongle',
    summary: 'A dedicated dongle is the central and sends keyboard and pointer input over USB.',
    controllers: Object.freeze([
      Object.freeze({ role: 'USB dongle · central', artifact: 'corney-usb-dongle.uf2' }),
      Object.freeze({ role: 'Left half · peripheral', artifact: 'corney-left-peripheral.uf2' }),
      Object.freeze({ role: 'Right half · peripheral', artifact: 'corney-right.uf2' }),
    ]),
  }),
]);

export const recovery = Object.freeze({
  artifact: 'settings-reset.uf2',
  role: 'Temporary settings reset',
});

// Update only after the exact GitHub Release assets and physical topology checks are accepted.
// Published records require version, sourceRevision, releaseUrl and all five SHA-256 values.
export const currentRelease = Object.freeze({ published: false });

const sha256Pattern = /^[a-f0-9]{64}$/;
const versionPattern = /^v[0-9]+\.[0-9]+\.[0-9]+(?:-[a-z0-9.-]+)?$/;

export function validateFirmwareData(release = currentRelease, topologyList = topologies) {
  const expected = new Set(artifactNames);
  if (expected.size !== 5 || !expected.has(recovery.artifact)) {
    throw new Error('Expected five distinct firmware artifacts, including settings reset');
  }
  const normal = new Set();
  for (const topology of topologyList) {
    if (!['direct', 'dongle'].includes(topology.id) || !topology.controllers.length) {
      throw new Error('Unknown or empty topology');
    }
    const local = new Set();
    for (const controller of topology.controllers) {
      if (!expected.has(controller.artifact) || controller.artifact === recovery.artifact) {
        throw new Error(`Invalid operating artifact: ${controller.artifact}`);
      }
      if (local.has(controller.artifact)) throw new Error(`Duplicate topology artifact: ${controller.artifact}`);
      local.add(controller.artifact);
      normal.add(controller.artifact);
    }
  }
  if (topologyList.length !== 2 || topologyList[0].id !== 'direct' || topologyList[1].id !== 'dongle' || normal.size !== 4) {
    throw new Error('Direct and dongle topologies must cover four operating images');
  }
  if (release.published !== true) {
    if (release.published !== false || Object.keys(release).some((key) => key !== 'published')) {
      throw new Error('Unpublished release must not include public download data');
    }
    return;
  }
  if (!versionPattern.test(release.version ?? '') || !/^[a-f0-9]{40}$/.test(release.sourceRevision ?? '') ||
      release.bluetoothName !== 'Corney' || release.physicallyAccepted !== true) {
    throw new Error('Published release needs a version, source revision, default name and acceptance');
  }
  const expectedUrl = `https://github.com/maxistar/corney/releases/tag/${release.version}`;
  if (release.releaseUrl !== expectedUrl) throw new Error('Release URL does not match version');
  if (!Array.isArray(release.artifacts) || release.artifacts.length !== 5) {
    throw new Error('Published release needs exactly five artifacts');
  }
  const names = new Set();
  for (const artifact of release.artifacts) {
    if (!expected.has(artifact.name) || names.has(artifact.name) || !sha256Pattern.test(artifact.sha256 ?? '')) {
      throw new Error(`Invalid release artifact: ${artifact.name}`);
    }
    names.add(artifact.name);
  }
  if (names.size !== expected.size) throw new Error('Published release artifact set is incomplete');
}

export function releaseView(release = currentRelease) {
  validateFirmwareData(release);
  if (!release.published) return Object.freeze({ published: false, label: 'No verified public firmware release yet.' });
  const assetBase = `https://github.com/maxistar/corney/releases/download/${release.version}`;
  return Object.freeze({
    published: true,
    label: `Accepted firmware ${release.version}`,
    version: release.version,
    releaseUrl: release.releaseUrl,
    bundleUrl: `${assetBase}/corney-firmware-${release.version}.zip`,
    checksumUrl: `${assetBase}/SHA256SUMS`,
    artifacts: Object.fromEntries(release.artifacts.map(({ name, sha256 }) => [name, Object.freeze({ sha256, url: `${assetBase}/${name}` })])),
  });
}
