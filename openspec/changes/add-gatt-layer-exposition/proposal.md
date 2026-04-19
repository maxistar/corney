# Change: Add GATT layer number exposition

## Why
Third-party desktop or mobile tools cannot currently read the active layer from the keyboard, which blocks visual layer indicators. Exposing the selected layer over BLE enables companion software to display the current layer state.

## What Changes
- Add an external ZMK/Zephyr module in this repo that exposes the active layer number over GATT.
- Subscribe the module to layer state changes so it notifies clients when the active layer changes.
- Add Corney build defaults and documentation describing the characteristic and module integration.

## Impact
- Affected specs: gatt-layer-exposition
- Affected code: repo-local ZMK module sources, Corney shield defaults, configuration docs
