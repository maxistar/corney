# Stock Corney Firmware Verification

## Build evidence

- Date: 2026-09-09
- Corney base revision: `7ae1c5aa34ed02f4291c56fbca6784ea72059609`
- ZMK revision: `241ff395` (`v0.2.1`)
- Build image: `zmkfirmware/zmk-build-arm:stable`
  (`sha256:edb1c953438c6f720ddb79c3762f3972013b7fbbaf4fff3592fc869983e7afc5`)
- Portable host contract and metadata tests: pass
- Native ZMK integration tests: pass
  - `combo-wrapper-ordinary`
  - `combo-wrapper-overlap`
  - `event-sources-both-halves`

The stock central resolved configuration contains:

```text
CONFIG_ZMK_BLE=y
CONFIG_BT_BAS=y
CONFIG_BT_DIS=y
# CONFIG_ZMK_GATT_LAYER_EXPOSITION is not set
```

`CONFIG_ZMK_KEYBOARD_HELPER_EXTENSION` is unavailable when its parent option is disabled, and no
Keyboard Helper GATT source (`gatt_service.c`, `layer_state.c`, `ble_transport.c`, or
`ble_event_sources.c`) appears in the stock build graph.

## CI-equivalent firmware set

The complete local reproduction of the build matrix produced the following files for the existing
GitHub Actions upload/merge path:

| Artifact | Size | SHA-256 |
| --- | ---: | --- |
| `corney-left-enhanced.uf2` | 540672 bytes | `20bc32050c30af0fe0c676af1bf6de493fb1f31f8f23874efb19ce73e8bc07ac` |
| `corney-left-stock.uf2` | 474112 bytes | `ade26410cb26e240c1cfe49084ec29b2b481b9a8ed8d5a00cadb9502830548a4` |
| `corney-left-legacy.uf2` | 475648 bytes | `6a12134800e02193b8d6135d83c7053cf362027551626d7240ebbb496c194a66` |
| `corney-left-extension-minimal.uf2` | 480768 bytes | `1a7c30ca3fa07324dd3fc1aab4f2e25e02a459cfe5f8846c922dedb655d153fb` |
| `corney-right.uf2` | 345600 bytes | `98fa37fa9590c33731f914ecb122d125cb87282a0fecf5a9d75f6c9b01d917c1` |
| `settings-reset.uf2` | 92672 bytes | `dcaf02725ce78d7ce03f598bdafed07e07411a02f7faed918be9c91f45f48d28` |

All six builds completed without a reported build error. The workflow's existing `artifact-*`
upload pattern includes every matrix row and merges those per-build outputs into `firmware`.

## Physical acceptance

Date: 2026-09-09

- `corney-left-stock.uf2` flashed successfully on the central nice!nano.
- Matching `corney-right.uf2` flashed successfully on the peripheral nice!nano.
- User-confirmed ordinary keyboard and split operation: pass.
- User-confirmed mobile stock-mode identification: pass.
- User-confirmed BLE GATT service discovery: pass.
  - Present standard services: `1800`, `1801`, `180f`, `180a`, `1812`.
  - Absent custom service: `b34a0001-e782-4706-8f9c-6c056c416507`.

The observed standard service subset matches the resolved build configuration with Battery Service
and Device Information Service enabled. No Keyboard Helper legacy, capability, or telemetry
characteristic can be exposed because their parent custom service is absent.
