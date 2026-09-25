# BlueShift components

Modular firmware boundaries. Milestone 2 adds host-testable logic and a
T-Lion board pin layer (**DOCUMENTED / IMPLEMENTED_UNVERIFIED**).

| Area | Status |
| --- | --- |
| `hardware/t_lion/` | DOCUMENTED pins from LilyGO schematic + `adc.ino` |
| `display/` | Abstraction + framebuffer mock |
| `input/` | 5-way navigation debounce / long-press |
| `battery/` | Non-linear % model |
| `power/` | Power policy (no aggressive sleep yet) |
| `hid/` | Normalized models + BLE report builders |
| `bridge/` | FSM + bounded queues |
| `bluetooth/` | Transport interfaces + profile placeholders (no stack deps) |
| `ui/` | Screen/nav model (i18x-ready) |
| `diagnostics/` | RAM diag session records |
| `storage/` | Versioned `AppConfig` |
| `i18n/` | en-US / de-DE OLED strings |
| `logging/` | (via `include/blueshift/log.h`) |

Third-party Bluetooth stacks stay behind project-owned interfaces.
