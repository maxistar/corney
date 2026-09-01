#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT

cc -std=c11 -Wall -Wextra -Werror -pedantic \
  -I"$repo_dir/include" \
  "$repo_dir/src/ble_contract.c" \
  "$repo_dir/src/ble_frame_history.c" \
  "$repo_dir/src/ble_subscriber_registry.c" \
  "$repo_dir/src/ble_transport_policy.c" \
  "$repo_dir/tests/ble_contract_test.c" \
  -o "$test_dir/ble-contract-test"

"$test_dir/ble-contract-test"
python3 "$repo_dir/tests/verify_ble_metadata.py"
