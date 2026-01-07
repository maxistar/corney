## Context
ZMK currently exposes BLE HID services, but not a dedicated characteristic for layer state. Companion apps need a lightweight, stable way to read the active layer number.

## Goals / Non-Goals
- Goals: Provide a GATT characteristic that returns the active layer number and optionally notifies on change.
- Non-Goals: Expose full layer names or keymaps, or provide bi-directional layer control.

## Decisions
- Decision: Use a dedicated custom GATT service and characteristic to avoid overloading HID semantics.
- Decision: Represent the layer as a zero-based unsigned integer corresponding to the highest active layer.
- Decision: Service UUID `715d81e1-377d-4f26-a678-a506675d99ec` and characteristic UUID `e87c518a-f323-4dd7-9dd5-0991add1c01b`.
- Alternatives considered: Advertising layer in BLE name (too noisy), HID feature reports (tooling complexity).

## Risks / Trade-offs
- Additional BLE characteristic increases GATT size and may impact power minimally.
- Companion apps must know the UUIDs; coordination/documentation is required.

## Migration Plan
- Introduce the characteristic behind a config flag enabled by default for this repo.
- Provide documentation and example UUIDs for companion apps.

## Open Questions
- Do we need to debounce rapid layer changes to avoid notification spam?
