#!/usr/bin/env python3

from pathlib import Path
import re
import sys


REPO = Path(__file__).resolve().parents[1]
HEADER = REPO / "include/corney/dongle_power_status.h"
SCREEN_SOURCE = REPO / "src/dongle_status_screen.c"
CMAKE = REPO / "CMakeLists.txt"
FONT_DIR = REPO / "modules/lib/gui/lvgl/src/font"
# Pinned ZMK v0.3.0 LVGL Montserrat metrics. The host-test job checks out
# Corney without running west update, so the external font sources are optional.
# When available, verify them against this committed fixture.
PINNED_FONT_METRICS = {
    12: ({" ": 52, "%": 162, "0": 128, "1": 71}, 15),
    16: ({}, 18),
}


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def define(name: str, source: str) -> int:
    match = re.search(rf"^#define {name} (\d+)U$", source, re.MULTILINE)
    if match is None:
        fail(f"missing numeric layout constant {name}")
    return int(match.group(1))


def font_metrics(size: int) -> tuple[dict[str, int], int]:
    expected_advances, expected_line_height = PINNED_FONT_METRICS[size]
    font_path = FONT_DIR / f"lv_font_montserrat_{size}.c"
    if not font_path.is_file():
        return expected_advances, expected_line_height

    source = font_path.read_text(encoding="utf-8")
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
    line_height = int(line_height_match.group(1))
    if line_height != expected_line_height:
        fail(f"Montserrat {size} line height differs from the pinned fixture")
    for character, expected_advance in expected_advances.items():
        glyph_id = ord(character) - 0x20 + 1
        if glyph_id >= len(advances) or advances[glyph_id] != expected_advance:
            fail(f"Montserrat {size} advance for {character!r} differs from the pinned fixture")
    return expected_advances, expected_line_height


def unkerned_ascii_width(text: str, advances: dict[str, int]) -> int:
    advance_units = 0
    for character in text:
        codepoint = ord(character)
        if not 0x20 <= codepoint <= 0x7F:
            fail(f"layout fixture contains unsupported character {character!r}")
        if character not in advances:
            fail(f"missing pinned advance for {character!r}")
        advance_units += advances[character]

    # LVGL stores advances in 1/16 pixel units. Ignoring kerning is a safe
    # upper bound for this numeric fixture in the pinned Montserrat font.
    return (advance_units + 15) // 16


def main() -> int:
    header = HEADER.read_text(encoding="utf-8")
    screen_source = SCREEN_SOURCE.read_text(encoding="utf-8")
    cmake_source = CMAKE.read_text(encoding="utf-8")
    display_width = define("CORNEY_DONGLE_DISPLAY_WIDTH_PX", header)
    display_height = define("CORNEY_DONGLE_DISPLAY_HEIGHT_PX", header)
    top_height = define("CORNEY_DONGLE_STOCK_TOP_HEIGHT_PX", header)
    bottom_height = define("CORNEY_DONGLE_STOCK_BOTTOM_HEIGHT_PX", header)
    remote_y = define("CORNEY_DONGLE_REMOTE_ROW_Y_PX", header)
    remote_height = define("CORNEY_DONGLE_REMOTE_ROW_HEIGHT_PX", header)
    remote_width = define("CORNEY_DONGLE_REMOTE_ROW_WIDTH_PX", header)
    layer_icon_width = define("CORNEY_DONGLE_LAYER_ICON_WIDTH_PX", header)
    layer_name_x = define("CORNEY_DONGLE_LAYER_NAME_X_PX", header)
    layer_name_width = define("CORNEY_DONGLE_LAYER_NAME_WIDTH_PX", header)
    layer_scroll_speed = define(
        "CORNEY_DONGLE_LAYER_SCROLL_SPEED_PX_PER_SEC", header
    )

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
        (
            layer_icon_width < layer_name_x,
            "layer name must leave a gap after the fixed keyboard symbol",
        ),
        (
            layer_name_x + layer_name_width == display_width,
            "layer name must use the remaining width through the right display edge",
        ),
        (
            "zmk/display/widgets/layer_status.h" not in screen_source
            and "zmk_widget_layer_status" not in screen_source,
            "dongle screen must not use the stock truncating layer widget",
        ),
        (
            "zmk_keymap_highest_layer_active()" in screen_source
            and "zmk_keymap_layer_name(layer)" in screen_source,
            "layer state must use ZMK's active layer and full display-name API",
        ),
        (
            "ZMK_SUBSCRIPTION(corney_dongle_layer_listener, zmk_layer_state_changed)"
            in screen_source,
            "layer-name widget must update on ZMK layer-state events",
        ),
        (
            "lv_label_set_long_mode(layer_name_label, LV_LABEL_LONG_SCROLL_CIRCULAR)"
            in screen_source,
            "layer name must use circular scrolling for overflow",
        ),
        (
            screen_source.index("layer_name_label = lv_label_create(screen);")
            < screen_source.index("corney_dongle_layer_listener_init();"),
            "layer label must exist before its display listener initializes",
        ),
        (
            "LV_SYMBOL_KEYBOARD" in screen_source,
            "layer row must retain its stationary keyboard symbol",
        ),
        (
            layer_scroll_speed == 20
            and "lv_obj_set_style_anim_speed(" in screen_source
            and "CORNEY_DONGLE_LAYER_SCROLL_SPEED_PX_PER_SEC" in screen_source,
            "long layer names must scroll at the reduced 20 px/s speed",
        ),
        (
            "target_sources_ifdef(CONFIG_CORNEY_DONGLE_BATTERY_STATUS_SCREEN app PRIVATE"
            in cmake_source
            and "src/dongle_status_screen.c" in cmake_source,
            "full-name layer widget must compile only with the dongle screen option",
        ),
    )

    failures = [message for condition, message in checks if not condition]
    if failures:
        for message in failures:
            print(f"FAIL: {message}", file=sys.stderr)
        return 1

    print(
        "Dongle display layout verified: "
        f"100% 100% <= {remote_width}px, layer name={layer_name_width}px at "
        f"{layer_scroll_speed}px/s, "
        f"remote y={remote_y}..{remote_y + remote_height - 1}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
