#!/bin/sh
set -eu

corney_repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
corney_zmk_source=${ZMK_SRC_DIR:-"$corney_repo_dir/zmk/app"}
corney_test_build=${ZMK_BUILD_DIR:-"$corney_repo_dir/build/native-tests"}

if [ ! -x "$corney_zmk_source/run-test.sh" ]; then
    echo "ZMK test runner not found at $corney_zmk_source/run-test.sh" >&2
    exit 1
fi

ZMK_SRC_DIR="$corney_zmk_source" \
ZMK_BUILD_DIR="$corney_test_build" \
ZMK_EXTRA_MODULES="$corney_repo_dir" \
    "$corney_zmk_source/run-test.sh" "$corney_repo_dir/tests/zmk"
