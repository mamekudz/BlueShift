# BlueShift components

Modular firmware boundaries for later milestones. Empty implementation folders
are intentional in milestone 1 — no placeholder classes merely to fill trees.

Planned ownership (see `CLAUDE.md`):

| Area | Responsibility |
| --- | --- |
| `hardware/` | Board bring-up, pins (after physical verification), power |
| `display/` | OLED abstraction (controller TBD until verified) |
| `input/` | Local buttons / navigation |
| `bluetooth/` | Classic HID host + BLE HID device adapters |
| `bridge/` | Normalized HID bridge orchestration |
| `diagnostics/` | Structured logging / diagnostic export |
| `storage/` | Persistent configuration |
| `ui/` | OLED screens / menus (via i18x strings) |
| `i18n/` | Firmware-facing i18x locale resources (started) |

Third-party Bluetooth stacks must stay behind project-owned interfaces
(`ClassicHidHost`, `BleHidDevice`, …) — not scattered through application code.
