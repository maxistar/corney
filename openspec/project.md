# Project Context

## Purpose
Maintain a ZMK firmware configuration for a Corne split keyboard using nice!nano v2 controllers, with a clean keymap and toggles for optional hardware (RGB underglow, OLED). Keep the layout simple for daily typing while preserving portability for firmware rebuilds via CI.

## Tech Stack
- ZMK firmware (Zephyr-based) via west workspace, targeting `nice_nano@2.0.0` with `corne_left` / `corne_right` shields
- DeviceTree overlays for keymap/layers (`config/corne.keymap`)
- Zephyr module config for board root discovery (`zephyr/module.yml`)
- GitHub Actions (`.github/workflows/build.yml`) reusing `zmkfirmware/zmk` build workflow and matrix from `build.yaml`

## Project Conventions

### Code Style
- DeviceTree formatting consistent with ZMK examples: layer blocks grouped under `keymap`, aligned columns for readability, and descriptive `display-name` per layer
- Keep optional features commented in `config/corne.conf` with short notes, favoring minimal diffs when toggling
- Use ZMK key behavior bindings (`&kp`, `&mo`, `&bt`) explicitly; avoid macros unless upstream recommends

### Architecture Patterns
- Single keymap file defining three layers: default typing, lower (numbers/navigation/Bluetooth select), and raise (symbols/punctuation)
- Layer switching uses momentary layers (`&mo 1`, `&mo 2`) on thumbs; thumbs also handle space/enter/modifiers
- Hardware options (RGB underglow, OLED) controlled via Kconfig flags in `config/corne.conf`
- Build matrix driven by `build.yaml` to produce separate firmware artifacts for left/right halves

### Testing Strategy
- Rely on GitHub Actions CI to compile both halves via upstream ZMK workflow
- Local validation via `west build -s config -b nice_nano@2.0.0 -- -DSHIELD=corne_left` (or `corne_right`) before pushing changes

### Git Workflow
- Use feature branches with pull requests; CI runs on push and PR to gate merges
- Keep commits small and descriptive (what/why); no enforced prefix scheme observed

## Domain Context
- Target hardware: Corne split keyboard with nice!nano v2 controllers; Bluetooth profile slots available via `&bt BT_SEL`
- Default layer is QWERTY; lower layer supplies numbers/navigation and Bluetooth controls; raise layer provides symbols/shifted punctuation
- Thumb keys: left thumb holds GUI + lower, right thumb enter + raise; escape and backspace occupy outer pinky positions on default layer

## Important Constraints
- Firmware size and power must fit within nice!nano v2 constraints; avoid unnecessary features when enabling underglow/display
- Stick to ZMK 0.3 tooling (west manifest default revision) unless intentionally upgrading
- Keep changes compatible with the upstream `corne_*` shields (no custom hardware overlays present)

## External Dependencies
- Upstream ZMK repository (`https://github.com/zmkfirmware/zmk`, imported via west)
- GitHub Actions workflow `zmkfirmware/zmk/.github/workflows/build-user-config.yml@v0.3`
- West toolchain + Zephyr SDK for local builds
