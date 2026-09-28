#!/usr/bin/env python3

import argparse
from pathlib import Path
import re
import sys


def read(path: str) -> str:
    return Path(path).read_text(encoding="utf-8")


def config_enabled(config: str, symbol: str) -> bool:
    return f"CONFIG_{symbol}=y" in config.splitlines()


def require(condition: bool, message: str, failures: list[str]) -> None:
    if not condition:
        failures.append(message)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify the generated Corney Cirque split topology"
    )
    parser.add_argument("--shield")
    parser.add_argument("--config")
    parser.add_argument("--dts")
    parser.add_argument("--build-yaml", default="build.yaml")
    parser.add_argument(
        "--matrix-only",
        action="store_true",
        help="verify only the declared release artifact matrix",
    )
    args = parser.parse_args()

    build_yaml = read(args.build_yaml)
    failures: list[str] = []

    artifact_names = re.findall(
        r"^\s*artifact-name:\s*(\S+)\s*$", build_yaml, re.MULTILINE
    )
    expected_artifact_names = [
        "corney-left-enhanced",
        "corney-right",
        "settings-reset",
    ]
    retired_artifact_names = {
        "corney-left-stock",
        "corney-left-legacy",
        "corney-left-extension-minimal",
    }

    require(
        artifact_names == expected_artifact_names,
        "build.yaml must declare exactly the enhanced left, universal right, and "
        f"settings-reset artifacts in release order; found {artifact_names}",
        failures,
    )
    require(
        retired_artifact_names.isdisjoint(artifact_names),
        "build.yaml must not declare retired stock, legacy, or extension-minimal artifacts",
        failures,
    )

    require(
        build_yaml.count("artifact-name: corney-right") == 1,
        "build.yaml must declare exactly one corney-right artifact",
        failures,
    )
    require(
        "studio-rpc-usb-uart" not in build_yaml,
        "build.yaml must not enable the ZMK Studio RPC snippet",
        failures,
    )
    require(
        build_yaml.count("-DCONFIG_ZMK_STUDIO=n") == 1,
        "build.yaml must explicitly disable ZMK Studio on the enhanced left build",
        failures,
    )
    require(
        "-DCONFIG_ZMK_STUDIO=y" not in build_yaml,
        "build.yaml must not enable ZMK Studio",
        failures,
    )
    require(
        build_yaml.count("-DCONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y") == 1,
        "build.yaml must retain the full Keyboard Helper extension on the enhanced left build",
        failures,
    )

    if args.matrix_only:
        if failures:
            for failure in failures:
                print(f"FAIL: {failure}", file=sys.stderr)
            return 1

        print("Corney release matrix verified")
        return 0

    missing_arguments = [
        option
        for option, value in (
            ("--shield", args.shield),
            ("--config", args.config),
            ("--dts", args.dts),
        )
        if value is None
    ]
    if missing_arguments:
        parser.error(
            "the following arguments are required without --matrix-only: "
            + ", ".join(missing_arguments)
        )

    config = read(args.config)
    dts = read(args.dts)
    has_driver = config_enabled(config, "INPUT_CORNEY_PINNACLE_POLLING")
    has_i2c = config_enabled(config, "I2C")
    has_input_split = config_enabled(config, "ZMK_INPUT_SPLIT")
    has_disconnect_release = config_enabled(
        config, "CORNEY_INPUT_SPLIT_DISCONNECT_RELEASE"
    )
    sensor_node_count = dts.count('compatible = "corney,cirque-pinnacle-polling";')
    has_sensor_node = sensor_node_count > 0
    has_split_node = 'compatible = "zmk,input-split";' in dts
    has_local_listener = "glidepoint_left_listener {" in dts
    has_proxy_listener = "glidepoint_split_listener {" in dts
    local_listener_references_sensor = "device = < &glidepoint_left >;" in dts
    proxy_listener_references_split = "device = < &glidepoint_split >;" in dts
    split_references_sensor = "device = < &glidepoint >;" in dts
    has_invert_x = "invert-x;" in dts
    has_invert_y = "invert-y;" in dts

    if args.shield == "corney_right":
        require(has_driver, "corney_right must enable the polling driver", failures)
        require(has_i2c, "corney_right must enable I2C", failures)
        require(has_input_split, "corney_right must enable input-split", failures)
        require(not has_disconnect_release, "disconnect release is central-only", failures)
        require(has_sensor_node, "corney_right must contain the Cirque node", failures)
        require(sensor_node_count == 1, "corney_right must contain one Cirque node", failures)
        require(has_split_node, "corney_right must contain the input-split endpoint", failures)
        require(
            split_references_sensor,
            "corney_right input-split endpoint must reference the Cirque device",
            failures,
        )
        require(not has_local_listener, "corney_right must not expose the left local listener", failures)
        require(not has_proxy_listener, "corney_right must not expose the proxy listener", failures)
        require(not has_invert_x, "180-degree right sensor must not invert X", failures)
        require(has_invert_y, "180-degree right sensor must invert Y", failures)
    elif args.shield == "corney_left":
        studio_symbols = (
            "ZMK_STUDIO",
            "ZMK_STUDIO_RPC",
            "ZMK_STUDIO_TRANSPORT_UART",
            "ZMK_STUDIO_TRANSPORT_BLE",
            "USB_CDC_ACM",
            "NANOPB",
            "ZMK_KEYMAP_SETTINGS_STORAGE",
        )
        helper_symbols = (
            "ZMK_KEYBOARD_HELPER_EXTENSION",
            "ZMK_KEYBOARD_HELPER_KEY_EVENTS",
            "ZMK_KEYBOARD_HELPER_COMBO_EVENTS",
            "ZMK_KEYBOARD_HELPER_LAYER_EVENTS",
            "ZMK_KEYBOARD_HELPER_DIAGNOSTICS",
        )

        for symbol in studio_symbols:
            require(
                not config_enabled(config, symbol),
                f"corney_left must not enable CONFIG_{symbol}",
                failures,
            )
        for symbol in helper_symbols:
            require(
                config_enabled(config, symbol),
                f"corney_left must retain CONFIG_{symbol}",
                failures,
            )
        require(config_enabled(config, "ZMK_USB"), "corney_left must retain USB HID", failures)
        require(config_enabled(config, "ZMK_BLE"), "corney_left must retain BLE HID", failures)
        require(has_driver, "corney_left must enable the polling driver", failures)
        require(has_i2c, "corney_left must enable Cirque I2C", failures)
        require(has_input_split, "corney_left must enable input-split", failures)
        require(has_disconnect_release, "corney_left must enable disconnect release", failures)
        require(has_sensor_node, "corney_left must contain the local Cirque node", failures)
        require(sensor_node_count == 1, "corney_left must contain one Cirque node", failures)
        require(has_split_node, "corney_left must contain the input-split proxy", failures)
        require(has_local_listener, "corney_left must listen to its local Cirque device", failures)
        require(has_proxy_listener, "corney_left must listen to the input-split proxy", failures)
        require(
            local_listener_references_sensor,
            "corney_left local listener must reference the local Cirque device",
            failures,
        )
        require(
            proxy_listener_references_split,
            "corney_left proxy listener must reference the input-split proxy",
            failures,
        )
        require(
            not split_references_sensor,
            "corney_left proxy must not reference a physical Cirque device",
            failures,
        )
        require(has_invert_x, "left sensor must invert X", failures)
        require(not has_invert_y, "left sensor must not invert Y", failures)
    elif args.shield == "settings_reset":
        require(not has_driver, "settings_reset must not enable the polling driver", failures)
        require(not has_sensor_node, "settings_reset must not contain the Cirque node", failures)
        require(not has_split_node, "settings_reset must not contain input-split", failures)
        require(not has_local_listener, "settings_reset must not contain a local listener", failures)
        require(not has_proxy_listener, "settings_reset must not contain the proxy listener", failures)
    else:
        failures.append(f"unsupported shield: {args.shield}")

    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        return 1

    print(f"Cirque topology verified for {args.shield}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
