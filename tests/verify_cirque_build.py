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
        "corney-left-peripheral",
        "corney-right",
        "corney-usb-dongle",
        "settings-reset",
    ]
    retired_artifact_names = {
        "corney-left-stock",
        "corney-left-legacy",
        "corney-left-extension-minimal",
    }

    require(
        artifact_names == expected_artifact_names,
        "build.yaml must declare exactly the direct central, left peripheral, "
        "universal right, dongle central, and settings-reset artifacts in release "
        f"order; found {artifact_names}",
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
        build_yaml.count("artifact-name: corney-left-peripheral") == 1,
        "build.yaml must declare exactly one corney-left-peripheral artifact",
        failures,
    )
    require(
        build_yaml.count("artifact-name: corney-usb-dongle") == 1,
        "build.yaml must declare exactly one corney-usb-dongle artifact",
        failures,
    )
    require(
        "studio-rpc-usb-uart" not in build_yaml,
        "build.yaml must not enable the ZMK Studio RPC snippet",
        failures,
    )
    require(
        build_yaml.count("-DCONFIG_ZMK_STUDIO=n") == 2,
        "build.yaml must explicitly disable ZMK Studio on both central builds",
        failures,
    )
    require(
        "-DCONFIG_ZMK_STUDIO=y" not in build_yaml,
        "build.yaml must not enable ZMK Studio",
        failures,
    )
    require(
        build_yaml.count("-DCONFIG_ZMK_KEYBOARD_HELPER_EXTENSION=y") == 2,
        "build.yaml must enable the full Keyboard Helper extension on both central builds",
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
    has_right_proxy_listener = "glidepoint_right_split_listener {" in dts
    has_left_proxy_listener = "glidepoint_left_split_listener {" in dts
    local_listener_references_sensor = "device = < &glidepoint_left >;" in dts
    proxy_listener_references_split = "device = < &glidepoint_split >;" in dts
    right_proxy_listener_references_split = (
        "device = < &glidepoint_right_split >;" in dts
    )
    left_proxy_listener_references_split = "device = < &glidepoint_left_split >;" in dts
    split_references_sensor = "device = < &glidepoint >;" in dts
    left_split_references_sensor = "device = < &glidepoint_left >;" in dts
    has_invert_x = "invert-x;" in dts
    has_invert_y = "invert-y;" in dts
    split_regs = sorted(
        int(value, 16)
        for value in re.findall(
            r'compatible = "zmk,input-split";\s+reg = < 0x([0-9a-f]+) >;',
            dts,
        )
    )
    has_mock_kscan = 'compatible = "zmk,kscan-mock";' in dts
    has_gpio_kscan = 'compatible = "zmk,kscan-gpio-matrix";' in dts
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

    require(
        config_enabled(config, "BOARD_NICE_NANO_V2"),
        f"{args.shield} must build for nice_nano_v2",
        failures,
    )

    if args.shield in ("corney_left_peripheral", "corney_right"):
        for symbol in studio_symbols + helper_symbols:
            require(
                not config_enabled(config, symbol),
                f"{args.shield} must not enable CONFIG_{symbol}",
                failures,
            )

    if args.shield == "corney_right":
        require(has_driver, "corney_right must enable the polling driver", failures)
        require(has_i2c, "corney_right must enable I2C", failures)
        require(has_input_split, "corney_right must enable input-split", failures)
        require(not has_disconnect_release, "disconnect release is central-only", failures)
        require(has_sensor_node, "corney_right must contain the Cirque node", failures)
        require(sensor_node_count == 1, "corney_right must contain one Cirque node", failures)
        require(has_split_node, "corney_right must contain the input-split endpoint", failures)
        require(split_regs == [0], "corney_right must use input-split register 0", failures)
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
        require(split_regs == [0], "corney_left must proxy input-split register 0", failures)
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
        require(not has_mock_kscan, "corney_left must not use mock kscan", failures)
        require(has_gpio_kscan, "corney_left must scan its physical matrix", failures)
        require(
            'CONFIG_ZMK_KEYBOARD_NAME="Corney"' in config,
            "corney_left must keep the default Corney identity",
            failures,
        )
    elif args.shield == "corney_left_peripheral":
        require(
            not config_enabled(config, "ZMK_SPLIT_ROLE_CENTRAL"),
            "corney_left_peripheral must remain a split peripheral",
            failures,
        )
        require(not config_enabled(config, "ZMK_USB"), "left peripheral must not expose USB HID", failures)
        require(has_driver, "left peripheral must enable the polling driver", failures)
        require(has_i2c, "left peripheral must enable Cirque I2C", failures)
        require(has_input_split, "left peripheral must enable input-split", failures)
        require(not has_disconnect_release, "disconnect release is central-only", failures)
        require(sensor_node_count == 1, "left peripheral must contain one Cirque node", failures)
        require(split_regs == [1], "left peripheral must use input-split register 1", failures)
        require(
            left_split_references_sensor,
            "left peripheral input-split endpoint must reference its Cirque device",
            failures,
        )
        require(not has_local_listener, "left peripheral must not expose a local listener", failures)
        require(not has_proxy_listener, "left peripheral must not expose a proxy listener", failures)
        require(has_invert_x, "left peripheral sensor must invert X", failures)
        require(not has_invert_y, "left peripheral sensor must not invert Y", failures)
        require(has_gpio_kscan, "left peripheral must scan its physical matrix", failures)
        require(not has_mock_kscan, "left peripheral must not use mock kscan", failures)
    elif args.shield == "corney_dongle":
        for symbol in studio_symbols:
            require(
                not config_enabled(config, symbol),
                f"corney_dongle must not enable CONFIG_{symbol}",
                failures,
            )
        for symbol in helper_symbols:
            require(
                config_enabled(config, symbol),
                f"corney_dongle must enable CONFIG_{symbol}",
                failures,
            )
        require(config_enabled(config, "ZMK_USB"), "dongle must expose USB HID", failures)
        require(config_enabled(config, "ZMK_BLE"), "dongle must retain BLE for split and Helper", failures)
        require(config_enabled(config, "ZMK_SPLIT_ROLE_CENTRAL"), "dongle must be central", failures)
        require(
            "CONFIG_ZMK_SPLIT_BLE_CENTRAL_PERIPHERALS=2" in config,
            "dongle must accept two BLE split peripherals",
            failures,
        )
        require("CONFIG_BT_MAX_CONN=7" in config, "dongle must reserve seven connections", failures)
        require("CONFIG_BT_MAX_PAIRED=7" in config, "dongle must reserve seven pairings", failures)
        require(has_disconnect_release, "dongle must enable disconnect release", failures)
        require(not has_driver, "dongle must not enable the physical Cirque driver", failures)
        require(not has_i2c, "dongle must not enable Cirque I2C", failures)
        require(sensor_node_count == 0, "dongle must not contain a Cirque node", failures)
        require(split_regs == [0, 1], "dongle must proxy input-split registers 0 and 1", failures)
        require(has_right_proxy_listener, "dongle must contain the right proxy listener", failures)
        require(has_left_proxy_listener, "dongle must contain the left proxy listener", failures)
        require(
            right_proxy_listener_references_split,
            "dongle right listener must reference the right proxy",
            failures,
        )
        require(
            left_proxy_listener_references_split,
            "dongle left listener must reference the left proxy",
            failures,
        )
        require(has_mock_kscan, "dongle must use mock kscan", failures)
        require(not has_gpio_kscan, "dongle must not scan a physical matrix", failures)
        require(
            'CONFIG_ZMK_KEYBOARD_NAME="Corney"' in config,
            "dongle must keep the default Corney identity",
            failures,
        )
    elif args.shield == "settings_reset":
        require(not has_driver, "settings_reset must not enable the polling driver", failures)
        require(not has_sensor_node, "settings_reset must not contain the Cirque node", failures)
        require(not has_split_node, "settings_reset must not contain input-split", failures)
        require(not has_local_listener, "settings_reset must not contain a local listener", failures)
        require(not has_proxy_listener, "settings_reset must not contain the proxy listener", failures)
        require(not has_right_proxy_listener, "settings_reset must not contain right proxy", failures)
        require(not has_left_proxy_listener, "settings_reset must not contain left proxy", failures)
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
