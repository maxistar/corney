#!/usr/bin/env python3

import argparse
from pathlib import Path
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
    parser.add_argument("--shield", required=True)
    parser.add_argument("--config", required=True)
    parser.add_argument("--dts", required=True)
    parser.add_argument("--build-yaml", default="build.yaml")
    args = parser.parse_args()

    config = read(args.config)
    dts = read(args.dts)
    build_yaml = read(args.build_yaml)
    failures: list[str] = []

    require(
        build_yaml.count("artifact-name: corney-right") == 1,
        "build.yaml must declare exactly one corney-right artifact",
        failures,
    )

    has_driver = config_enabled(config, "INPUT_CORNEY_PINNACLE_POLLING")
    has_i2c = config_enabled(config, "I2C")
    has_input_split = config_enabled(config, "ZMK_INPUT_SPLIT")
    has_disconnect_release = config_enabled(
        config, "CORNEY_INPUT_SPLIT_DISCONNECT_RELEASE"
    )
    has_sensor_node = 'compatible = "corney,cirque-pinnacle-polling";' in dts
    has_split_node = 'compatible = "zmk,input-split";' in dts
    has_proxy_listener = "glidepoint_listener {" in dts
    split_references_sensor = "device = < &glidepoint >;" in dts
    has_invert_x = "invert-x;" in dts
    has_invert_y = "invert-y;" in dts

    if args.shield == "corney_right":
        require(has_driver, "corney_right must enable the polling driver", failures)
        require(has_i2c, "corney_right must enable I2C", failures)
        require(has_input_split, "corney_right must enable input-split", failures)
        require(not has_disconnect_release, "disconnect release is central-only", failures)
        require(has_sensor_node, "corney_right must contain the Cirque node", failures)
        require(has_split_node, "corney_right must contain the input-split endpoint", failures)
        require(
            split_references_sensor,
            "corney_right input-split endpoint must reference the Cirque device",
            failures,
        )
        require(not has_proxy_listener, "corney_right must not expose a local listener", failures)
        require(not has_invert_x, "180-degree right sensor must not invert X", failures)
        require(has_invert_y, "180-degree right sensor must invert Y", failures)
    elif args.shield == "corney_left":
        require(not has_driver, "corney_left must not enable the polling driver", failures)
        require(not has_i2c, "corney_left must not enable Cirque I2C", failures)
        require(has_input_split, "corney_left must enable input-split", failures)
        require(has_disconnect_release, "corney_left must enable disconnect release", failures)
        require(not has_sensor_node, "corney_left must not contain the Cirque node", failures)
        require(has_split_node, "corney_left must contain the input-split proxy", failures)
        require(has_proxy_listener, "corney_left must listen to the input-split proxy", failures)
        require(
            not split_references_sensor,
            "corney_left proxy must not reference a physical Cirque device",
            failures,
        )
    elif args.shield == "settings_reset":
        require(not has_driver, "settings_reset must not enable the polling driver", failures)
        require(not has_sensor_node, "settings_reset must not contain the Cirque node", failures)
        require(not has_split_node, "settings_reset must not contain input-split", failures)
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
