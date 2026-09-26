# Memory budget — BlueShift

**Status:** BUILD MEASURED where noted; runtime free heap **not** claimed until physical execution.  
**PHYSICAL VERIFIED: none.**

## Target

LILYGO T-Lion — ESP32-WROVER with PSRAM (DOCUMENTED module / ASSUMED 8 MB PSRAM).

## PSRAM policy

Core Bluetooth bridging must **not** depend on PSRAM unless evidence proves otherwise.

- Prefer internal DRAM for HID queues, bridge state, OLED 1 KiB framebuffer.
- PSRAM may later hold non-critical diagnostics/UI buffers.
- Firmware should continue if PSRAM is unavailable or disabled.

## Static / linker allocations (order of magnitude)

| Item | Size | Notes |
| --- | --- | --- |
| SSD1306 framebuffer | 1024 B | project-owned IDF backend |
| Bridge / UI / config / device meta | few KiB | BSS + stacks ASSUMED |
| HID report temps | tens of bytes | no per-report heap |
| Bounded queues | fixed capacity | DropOldest / KeepNewest |

## Latest build numbers (LINKER / BUILD MEASURED)

| Env | Flash used | DRAM used | Notes | Measured |
| --- | --- | --- | --- | --- |
| skeleton (M4 adapters, BT gated off) | 291873 / 1310720 (22.3%) | 23184 / 327680 (7.1%) | Arduino LEGACY | 2026-09-25 |
| t-lion-debug (M4 + BTDM platform) | 1137381 / 1310720 (86.8%) | 41444 / 327680 (12.6%) | Arduino LEGACY | 2026-09-25 |
| t-lion-idf-spike (dual) | 914852 / 1048576 (87.2%) | 44476 / 327680 (13.6%) | 1 MiB spike slot | 2026-09-26 |
| **t-lion-idf-debug** | **1026875 / 2228224 (46.1%)** | **48048 / 327680 (14.7%)** | production candidate | 2026-09-26 |
| **t-lion-idf-release** | **852596 / 1966080 (43.4%)** | **46392 / 327680 (14.2%)** | production candidate | 2026-09-26 |

## Runtime (pending physical)

| Item | Status |
| --- | --- |
| free heap | DETECTED at boot via `captureChipInfo` — claim only after board |
| min free heap | same |
| Bluedroid runtime heap | UNKNOWN until bring-up |

## Policy

- Prefer fixed buffers and bounded queues.
- No heap allocation in HID callbacks.
- One app loop task (`bs_app`) — not one task per component.
