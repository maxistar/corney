#!/usr/bin/env python3

from pathlib import Path
import re
import sys


REPO = Path(__file__).resolve().parents[1]
HEADER = REPO / "include/corney/dongle_power_status.h"
FONT_DIR = REPO / "modules/lib/gui/lvgl/src/font"


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def define(name: str, source: str) -> int:
    match = re.search(rf"^#define {name} (\d+)U$", source, re.MULTILINE)
    if match is None:
        fail(f"missing numeric layout constant {name}")
    return int(match.group(1))


def font_metrics(size: int) -> tuple[list[int], int]:
    source = (FONT_DIR / f"lv_font_montserrat_{size}.c").read_text(
        encoding="utf-8"
    )
    descriptor_block = re.search(
        r"glyph_dsc\[\] = \{(?P<body>.*?)\n\};", source, re.DOTALL
    )
    if descriptor_block is None:
        fail(f"cannot locate Montserrat {size} glyph descriptors")

    advances = [
        int(value)
        for value in re.findall(r"\.adv_w = (\d+)", descriptor_block.group("body"))
    ]
    line_height_match = re.search(r"\.line_height = (\d+)", source)
    if line_height_match is None:
        fail(f"cannot locate Montserrat {size} line height")
    return advances, int(line_height_match.group(1))


def unkerned_ascii_width(text: str, advances: list[int]) -> int:
    advance_units = 0
    for character in text:
        codepoint = ord(character)
        if not 0x20 <= codepoint <= 0x7F:
            fail(f"layout fixture contains unsupported character {character!r}")
        glyph_id = codepoint - 0x20 + 1
        advance_units += advances[glyph_id]

    # LVGL stores advances in 1/16 pixel units. Ignoring kerning is a safe
    # upper bound for this numeric fixture in the pinned Montserrat font.
    return (advance_units + 15) // 16


def main() -> int:
    header = HEADER.read_text(encoding="utf-8")
    display_width = define("CORNEY_DONGLE_DISPLAY_WIDTH_PX", header)
    display_height = define("CORNEY_DONGLE_DISPLAY_HEIGHT_PX", header)
    top_height = define("CORNEY_DONGLE_STOCK_TOP_HEIGHT_PX", header)
    bottom_height = define("CORNEY_DONGLE_STOCK_BOTTOM_HEIGHT_PX", header)
    remote_y = define("CORNEY_DONGLE_REMOTE_ROW_Y_PX", header)
    remote_height = define("CORNEY_DONGLE_REMOTE_ROW_HEIGHT_PX", header)
    remote_width = define("CORNEY_DONGLE_REMOTE_ROW_WIDTH_PX", header)

    font_12_advances, font_12_height = font_metrics(12)
    _, font_16_height = font_metrics(16)
    widest_pair_width = unkerned_ascii_width("100% 100%", font_12_advances)

    checks = (
        (display_width == 128 and display_height == 64, "display bounds must remain 128x64"),
        (top_height == font_16_height, "stock top band must match Montserrat 16"),
        (remote_height == font_12_height, "remote row must match Montserrat 12"),
        (bottom_height == font_12_height, "stock bottom band must match the small font"),
        (top_height <= remote_y, "remote row overlaps the stock top band"),
        (
            remote_y + remote_height <= display_height - bottom_height,
            "remote row overlaps the stock bottom band",
        ),
        (
            widest_pair_width <= remote_width,
            f"100% 100% needs {widest_pair_width}px but row is {remote_width}px",
        ),
        (remote_width <= display_width, "remote row exceeds the framebuffer width"),
    )

    failures = [message for condition, message in checks if not condition]
    if failures:
        for message in failures:
            print(f"FAIL: {message}", file=sys.stderr)
        return 1

    print(
        "Dongle display layout verified: "
        f"100% 100% <= {remote_width}px, y={remote_y}..{remote_y + remote_height - 1}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
