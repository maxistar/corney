"""Release inputs must be one complete, default-name compatibility set."""

import json
import hashlib
import subprocess
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "package_firmware_release.py"
ARTIFACTS = (
    "corney-left-enhanced.uf2",
    "corney-left-peripheral.uf2",
    "corney-right.uf2",
    "corney-usb-dongle.uf2",
    "settings-reset.uf2",
)
SHA = "a" * 40
TAG = "v1.2.3"
RUN = "123456"


class FirmwareReleaseTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.inputs = self.root / "inputs"
        self.inputs.mkdir()
        self.output = self.root / "output"
        self.config = self.root / "config"
        for index, name in enumerate(ARTIFACTS):
            (self.inputs / name).write_bytes(bytes([index + 1]) * 128)
            self.config.write_text(f'CONFIG_ZMK_KEYBOARD_NAME="{"Corney" if name in ARTIFACTS[:1] + ARTIFACTS[3:4] else ""}"\n')
            self.run_script(
                "provenance", "--artifact", name,
                "--image", str(self.inputs / name),
                "--config", str(self.config),
                "--output", str(self.inputs / f"{name}.provenance.json"),
            )

    def run_script(self, command, *args, success=True):
        result = subprocess.run(
            [sys.executable, str(SCRIPT), command, *args,
             "--tag", TAG, "--source-sha", SHA, "--run-id", RUN],
            capture_output=True, text=True, check=False,
        )
        if success:
            self.assertEqual(result.returncode, 0, result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout)
        return result

    def package(self, success=True):
        return self.run_script("package", "--input", str(self.inputs), "--output", str(self.output), success=success)

    def test_bundle_and_checksums_match_exact_set(self):
        self.package()
        manifest = (self.output / "SHA256SUMS").read_text().splitlines()
        self.assertEqual([line.split("  ")[1] for line in manifest], list(ARTIFACTS))
        with zipfile.ZipFile(self.output / f"corney-firmware-{TAG}.zip") as bundle:
            self.assertEqual(bundle.namelist(), [*ARTIFACTS, "SHA256SUMS"])
            for name in ARTIFACTS:
                self.assertEqual(bundle.read(name), (self.inputs / name).read_bytes())
        self.assertIn(SHA, (self.output / "RELEASE_NOTES.md").read_text())
        second = self.root / "second-output"
        self.run_script("package", "--input", str(self.inputs), "--output", str(second))
        self.assertEqual(
            hashlib.sha256((self.output / f"corney-firmware-{TAG}.zip").read_bytes()).hexdigest(),
            hashlib.sha256((second / f"corney-firmware-{TAG}.zip").read_bytes()).hexdigest(),
        )
        self.package(success=False)  # Existing release assets are never overwritten.

    def test_missing_extra_and_renamed_inputs_fail(self):
        (self.inputs / ARTIFACTS[1]).unlink()
        self.package(success=False)
        (self.inputs / ARTIFACTS[1]).write_bytes(b"replacement")
        (self.inputs / "unexpected.uf2").write_bytes(b"extra")
        self.package(success=False)

    def test_mixed_source_and_custom_name_fail(self):
        record_path = self.inputs / f"{ARTIFACTS[0]}.provenance.json"
        record = json.loads(record_path.read_text())
        record["source_sha"] = "b" * 40
        record_path.write_text(json.dumps(record))
        self.package(success=False)
        record["source_sha"] = SHA
        record["bluetooth_name"] = "CorneyMX"
        record_path.write_text(json.dumps(record))
        self.package(success=False)

    def test_checksum_and_duplicate_identity_fail(self):
        (self.inputs / ARTIFACTS[2]).write_bytes(b"tampered")
        self.package(success=False)
        record_path = self.inputs / f"{ARTIFACTS[2]}.provenance.json"
        record = json.loads(record_path.read_text())
        record["artifact"] = ARTIFACTS[0]
        record_path.write_text(json.dumps(record))
        self.package(success=False)

    def test_custom_name_is_rejected_before_provenance(self):
        self.config.write_text('CONFIG_ZMK_KEYBOARD_NAME="CorneyMX"\n')
        result = self.run_script(
            "provenance", "--artifact", ARTIFACTS[0],
            "--image", str(self.inputs / ARTIFACTS[0]),
            "--config", str(self.config),
            "--output", str(self.inputs / "bad.json"), success=False,
        )
        self.assertIn("default Corney", result.stderr)


if __name__ == "__main__":
    unittest.main()
