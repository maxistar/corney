import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { resolve, join, extname } from 'node:path';
import { artifactNames, currentRelease, recovery, releaseView, topologies, validateFirmwareData } from '../src/data/firmware.js';

const root = resolve(import.meta.dirname, '..');
const dist = join(root, 'dist');
const routes = ['', 'firmware/', 'build/', 'guide/'];
const pages = new Map();
const errors = [];
const fail = (message) => errors.push(message);
const decode = (value) => value.replaceAll('&amp;', '&');

validateFirmwareData(currentRelease);
for (const route of routes) {
  const path = join(dist, route, 'index.html');
  if (!existsSync(path)) { fail(`Missing route /corney/${route}`); continue; }
  const html = readFileSync(path, 'utf8');
  pages.set(route, html);
  if (!html.includes('<main id="main"')) fail(`Missing main landmark: ${route}`);
  if (!html.includes('href="#main"')) fail(`Missing skip link: ${route}`);
  if (!/<meta name="description" content="[^"]+"/.test(html)) fail(`Missing description: ${route}`);
  if (/<script\b/i.test(html)) fail(`Unexpected client script on static page: ${route}`);
  for (const [, , rawUrl] of html.matchAll(/<(a|link|img)\b[^>]*?\b(?:href|src)="([^"]+)"[^>]*>/g)) {
    const url = decode(rawUrl);
    if (url.startsWith('#')) {
      if (!html.includes(`id="${url.slice(1)}"`)) fail(`Broken fragment ${url} on ${route}`);
    } else if (url.startsWith('/')) {
      if (!url.startsWith('/corney/')) { fail(`Link escapes base: ${url}`); continue; }
      const relative = url.slice('/corney/'.length).split(/[?#]/)[0];
      const target = relative.endsWith('/') || !extname(relative) ? join(dist, relative, 'index.html') : join(dist, relative);
      if (!existsSync(target)) fail(`Missing local target: ${url}`);
    } else if (!/^(https?:|mailto:)/.test(url)) {
      fail(`Unsupported relative link on ${route}: ${url}`);
    }
  }
  for (const tag of html.match(/<img\b[^>]*>/g) ?? []) {
    if (!/\balt="[^"]+"/.test(tag)) fail(`Informative image lacks alt on ${route}`);
  }
  if (/\.artifacts\/|\/home\/maxim\/|node_modules\//.test(html)) fail(`Source path leaked on ${route}`);
}

const firmware = pages.get('firmware/') ?? '';
const guide = pages.get('guide/') ?? '';
for (const topology of topologies) {
  const section = firmware.match(new RegExp(`<section[^>]+id="${topology.id}"[\\s\\S]*?(?=<section|$)`))?.[0] ?? '';
  if (!section) fail(`Missing topology section ${topology.id}`);
  for (const { role, artifact } of topology.controllers) {
    if (!section.includes(role) || !section.includes(artifact)) fail(`Missing ${role} -> ${artifact} mapping`);
  }
}
if (!firmware.includes('id="recovery"') || !firmware.includes(recovery.artifact) || !firmware.includes('erases stored ZMK settings') || !firmware.includes('operating UF2 again')) {
  fail('Missing separate, warned recovery guidance');
}
if (!guide.includes('invalidates existing split bonds') || !guide.includes('genuine remote')) {
  fail('Guide lacks topology-switching or OLED limitation');
}
const view = releaseView();
if (!view.published) {
  if (!firmware.includes(view.label)) fail('Unpublished state is absent');
  if (/releases\/download\//.test(firmware) || /Download UF2|Download bundle/.test(firmware)) fail('Unpublished page exposes downloads');
} else {
  if (!firmware.includes(view.version) || !firmware.includes(view.bundleUrl) || !firmware.includes(view.checksumUrl)) fail('Release links absent');
  for (const name of artifactNames) {
    if (!firmware.includes(view.artifacts[name].url) || !firmware.includes(view.artifacts[name].sha256)) fail(`Missing download or checksum for ${name}`);
  }
}

for (const image of readdirSync(join(dist, 'images'))) {
  const path = join(dist, 'images', image);
  if (!/\.(?:webp|png|jpe?g)$/i.test(image)) continue;
  const bytes = readFileSync(path);
  if (image.endsWith('.webp')) {
    if (bytes.toString('ascii', 0, 4) !== 'RIFF' || bytes.toString('ascii', 8, 12) !== 'WEBP') fail(`Invalid WebP: ${image}`);
    let offset = 12;
    while (offset + 8 <= bytes.length) {
      const chunk = bytes.toString('ascii', offset, offset + 4);
      const size = bytes.readUInt32LE(offset + 4);
      if (chunk === 'EXIF' || chunk === 'XMP ') fail(`Metadata chunk retained: ${image}`);
      offset += 8 + size + (size % 2);
    }
  } else if (bytes.includes(Buffer.from('Exif')) || bytes.includes(Buffer.from('GPSInfo'))) {
    fail(`Potential EXIF metadata retained: ${image}`);
  }
}

if (errors.length) {
  for (const error of errors) console.error(`✗ ${error}`);
  process.exitCode = 1;
} else {
  console.log(`Validated ${pages.size} routes, both firmware topologies, release state, assets and public image metadata.`);
}
