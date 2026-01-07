# Change: Add GATT layer number exposition

## Why
Third-party desktop or mobile tools cannot currently read the active layer from the keyboard, which blocks visual layer indicators. Exposing the selected layer over BLE enables companion software to display the current layer state.

## What Changes
- Add a GATT characteristic that exposes the active layer number.
- Update the layer state pipeline to notify subscribed clients when the layer changes.
- Add configuration defaults and documentation describing the characteristic.

## Impact
- Affected specs: gatt-layer-exposition
- Affected code: ZMK BLE GATT services, layer state logic, configuration docs
