#!/usr/bin/env python3
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
LAYOUT = ROOT / "layout_corney.json"
MACROS = ROOT / "config/boards/shields/corney/corney_macros.dtsi"

EXPECTED = {
    1: ("combo_esc", [1, 2]),
    2: ("combo_enter", [15, 16]),
    3: ("combo_left_enter", [19, 20]),
    4: ("combo_delete", [27, 28]),
    5: ("combo_left_delete", [31, 32]),
    6: ("combo_mouse_layer", [40, 41]),
    7: ("combo_mouse_layer_left", [36, 37]),
}


def fail(message: str) -> None:
    print(f"BLE metadata error: {message}", file=sys.stderr)
    raise SystemExit(1)


layout = json.loads(LAYOUT.read_text(encoding="utf-8"))
if len(layout.get("keyPositions", [])) != 42:
    fail("position schema 1 requires exactly 42 keyPositions")

layout_combos = {combo.get("id"): combo.get("positions") for combo in layout.get("combos", [])}
if set(layout_combos) != set(EXPECTED):
    fail(f"layout combo IDs differ: {sorted(layout_combos)}")
if len(layout_combos) != len(layout.get("combos", [])):
    fail("layout combo IDs are not unique")

source = MACROS.read_text(encoding="utf-8")
for combo_id, (node, positions) in EXPECTED.items():
    if layout_combos[combo_id] != positions:
        fail(f"layout combo {combo_id} positions are {layout_combos[combo_id]}, expected {positions}")
    combo_pattern = rf"{node}\s*\{{.*?key-positions\s*=\s*<{' '.join(map(str, positions))}>;.*?bindings\s*=\s*<&kh_{node}>;"
    if re.search(combo_pattern, source, re.DOTALL) is None:
        fail(f"firmware combo node {node} does not match ID {combo_id} positions {positions}")
    wrapper_pattern = rf"kh_{node}:\s*kh_{node}\s*\{{.*?combo-id\s*=\s*<{combo_id}>;.*?key-positions\s*=\s*<{' '.join(map(str, positions))}>;"
    if re.search(wrapper_pattern, source, re.DOTALL) is None:
        fail(f"telemetry wrapper kh_{node} does not match ID {combo_id}")

print("BLE position/combo metadata is complete and consistent")
