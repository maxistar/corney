#!/usr/bin/env python3
"""Record provenance and package one verified five-image Corney release."""

import argparse
import hashlib
import json
import re
import shutil
import sys
import zipfile
from pathlib import Path

ARTIFACTS = (
    "corney-left-enhanced.uf2",
    "corney-left-peripheral.uf2",
    "corney-right.uf2",
    "corney-usb-dongle.uf2",
    "settings-reset.uf2",
)
HOST_IMAGES = {"corney-left-enhanced.uf2", "corney-usb-dongle.uf2"}
TAG_PATTERN = re.compile(r"v[0-9]+\.[0-9]+\.[0-9]+(?:-[a-z0-9.-]+)?\Z")
SHA_PATTERN = re.compile(r"[a-f0-9]{40}\Z")


def checksum(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for block in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def require_tag_and_sha(tag: str, source_sha: str) -> None:
    if not TAG_PATTERN.fullmatch(tag) or not SHA_PATTERN.fullmatch(source_sha):
        raise ValueError("Expected a vX.Y.Z tag and full lowercase source SHA")


def record_provenance(args: argparse.Namespace) -> None:
    require_tag_and_sha(args.tag, args.source_sha)
    if args.artifact not in ARTIFACTS or args.image.name != args.artifact:
        raise ValueError("Artifact identity does not match the UF2 file")
    if not args.image.is_file() or not args.config.is_file() or not args.run_id.isdigit():
        raise ValueError("Image, generated config and numeric run ID are required")
    config = args.config.read_text()
    match = re.search(r'^CONFIG_ZMK_KEYBOARD_NAME="([^"]*)"$', config, re.MULTILINE)
    if not match:
        raise ValueError("Generated keyboard name is missing")
    bluetooth_name = match.group(1)
    if args.artifact in HOST_IMAGES and bluetooth_name != "Corney":
        raise ValueError("Public central image must use the default Corney Bluetooth name")
    data = {
        "artifact": args.artifact,
        "source_sha": args.source_sha,
        "tag": args.tag,
        "run_id": args.run_id,
        "bluetooth_name": bluetooth_name,
        "sha256": checksum(args.image),
    }
    args.output.write_text(json.dumps(data, sort_keys=True, indent=2) + "\n")


def verify_inputs(input_dir: Path, tag: str, source_sha: str, run_id: str) -> dict[str, Path]:
    require_tag_and_sha(tag, source_sha)
    if not input_dir.is_dir() or not run_id.isdigit():
        raise ValueError("Input directory and numeric run ID are required")
    expected = {name for name in ARTIFACTS} | {f"{name}.provenance.json" for name in ARTIFACTS}
    actual = {item.name for item in input_dir.iterdir()}
    if actual != expected:
        raise ValueError(f"Expected exactly five UF2 files and five provenance records; missing={sorted(expected - actual)}, extra={sorted(actual - expected)}")
    images = {}
    for name in ARTIFACTS:
        image = input_dir / name
        record = json.loads((input_dir / f"{name}.provenance.json").read_text())
        if not image.is_file() or not isinstance(record, dict):
            raise ValueError(f"Invalid image or provenance: {name}")
        if record.get("artifact") != name or record.get("tag") != tag or record.get("source_sha") != source_sha or record.get("run_id") != run_id:
            raise ValueError(f"Mixed or invalid source provenance: {name}")
        if name in HOST_IMAGES and record.get("bluetooth_name") != "Corney":
            raise ValueError(f"Custom Bluetooth name in public central: {name}")
        if record.get("sha256") != checksum(image):
            raise ValueError(f"Checksum mismatch: {name}")
        images[name] = image
    return images


def package(args: argparse.Namespace) -> None:
    images = verify_inputs(args.input, args.tag, args.source_sha, args.run_id)
    if args.output.exists() and any(args.output.iterdir()):
        raise ValueError("Output directory must be empty; existing release assets are never overwritten")
    args.output.mkdir(parents=True, exist_ok=True)
    for name in ARTIFACTS:
        shutil.copyfile(images[name], args.output / name)
    manifest = "".join(f"{checksum(args.output / name)}  {name}\n" for name in ARTIFACTS)
    (args.output / "SHA256SUMS").write_text(manifest)
    bundle = args.output / f"corney-firmware-{args.tag}.zip"
    with zipfile.ZipFile(bundle, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name in (*ARTIFACTS, "SHA256SUMS"):
            entry = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            entry.compress_type = zipfile.ZIP_DEFLATED
            entry.external_attr = 0o644 << 16
            archive.writestr(entry, (args.output / name).read_bytes(), compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
    with zipfile.ZipFile(bundle) as archive:
        if archive.namelist() != [*ARTIFACTS, "SHA256SUMS"]:
            raise ValueError("Bundle contents do not match the five-image release")
        for name in ARTIFACTS:
            if hashlib.sha256(archive.read(name)).hexdigest() != checksum(args.output / name):
                raise ValueError(f"Bundle verification failed: {name}")
        if archive.read("SHA256SUMS").decode() != manifest:
            raise ValueError("Bundle checksum manifest mismatch")
    notes = (
        f"Corney firmware {args.tag}\n\n"
        f"Source revision: `{args.source_sha}`\n"
        f"GitHub Actions run: `{args.run_id}`\n"
        "Bluetooth name on host-facing central images: `Corney`.\n\n"
        "All five UF2 images are one compatibility set. Select Direct Bluetooth or USB Dongle "
        "before flashing. `settings-reset.uf2` clears stored bonds and must be followed by an "
        "operating image. Verify individual downloads with `SHA256SUMS`.\n"
    )
    (args.output / "RELEASE_NOTES.md").write_text(notes)
    print(f"Packaged {args.tag}: five verified UF2 files, SHA256SUMS and {bundle.name}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    provenance = sub.add_parser("provenance")
    provenance.add_argument("--artifact", required=True)
    provenance.add_argument("--image", required=True, type=Path)
    provenance.add_argument("--config", required=True, type=Path)
    provenance.add_argument("--output", required=True, type=Path)
    bundle = sub.add_parser("package")
    bundle.add_argument("--input", required=True, type=Path)
    bundle.add_argument("--output", required=True, type=Path)
    for command in (provenance, bundle):
        command.add_argument("--tag", required=True)
        command.add_argument("--source-sha", required=True)
        command.add_argument("--run-id", required=True)
    return parser.parse_args()


if __name__ == "__main__":
    try:
        arguments = parse_args()
        (record_provenance if arguments.command == "provenance" else package)(arguments)
    except (ValueError, OSError, json.JSONDecodeError) as error:
        print(f"Release packaging failed: {error}", file=sys.stderr)
        sys.exit(1)
