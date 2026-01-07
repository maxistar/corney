## Context
ZMK currently exposes BLE HID services, but not a dedicated characteristic for layer state. Companion apps need a lightweight, stable way to read the active layer number.

## Goals / Non-Goals
- Goals: Provide a GATT characteristic that returns the active layer number and optionally notifies on change.
- Non-Goals: Expose full layer names or keymaps, or provide bi-directional layer control.

## Decisions
- Decision: Use a dedicated custom GATT service and characteristic to avoid overloading HID semantics.
- Decision: Represent the layer as a zero-based unsigned integer corresponding to the highest active layer.
- Alternatives considered: Advertising layer in BLE name (too noisy), HID feature reports (tooling complexity).

## Risks / Trade-offs
- Additional BLE characteristic increases GATT size and may impact power minimally.
- Companion apps must know the UUIDs; coordination/documentation is required.

## Migration Plan
- Introduce the characteristic behind a config flag enabled by default for this repo.
- Provide documentation and example UUIDs for companion apps.

## Open Questions
- Should the characteristic be under an existing ZMK custom service or a new service UUID?
- Do we need to debounce rapid layer changes to avoid notification spam?
