# Memory budget (Milestone 3)

**Status:** ESTIMATED / MEASURED-AT-BUILD where PlatformIO size output is available.
Nothing here is a physical runtime claim.

## Target

LILYGO T-Lion — ESP32-WROVER with PSRAM (DOCUMENTED / IMPLEMENTED_UNVERIFIED board JSON).

## Methodology

1. Build `t-lion-debug` / `skeleton` and capture `.pio` size summary.
2. Compare flash / DRAM before vs after Bluetooth dependencies.
3. Do not enable NimBLE + Classic Bluedroid dual-host until EspBle/Arduino 3.x path is chosen.

## Static application allocations (order of magnitude)

| Item | Size | Notes |
| --- | --- | --- |
| SSD1306 framebuffer (library-owned) | ~1 KiB | 128×64 / 8 |
| `FramebufferDisplay` host mirror | 1024 B | tests / skeleton |
| Bridge / UI / config objects | < 2 KiB | stack + BSS ASSUMED |
| HID report temps | tens of bytes | no per-report heap |

## Dependency flash impact (EXPECTED)

| Configuration | Flash impact | Status |
| --- | --- | --- |
| App + ThingPulse SSD1306 | moderate | linked on `t-lion-*` |
| + NimBLE-Arduino HID | large (100s of KiB) | **not default** — optional flag |
| + EspBle dual-host | very large | blocked on Arduino 2.x / platform 6.9.0 |
| + Bluepad32/BTstack | very large | not linked |

## PSRAM

`BOARD_HAS_PSRAM` set for T-Lion envs. Bluetooth stacks may or may not use PSRAM — ASSUMED until measurement.

## Policy

- Prefer fixed buffers and bounded queues.
- No heap allocation in HID callbacks.
- If a stack does not fit, stop integration and keep mocks/core/UI (see CLAUDE.md / Milestone 3 §41–42).

## Latest build numbers

| Env | Flash used | DRAM used | Notes | Measured |
| --- | --- | --- | --- | --- |
| skeleton (pre-M4) | 290569 / 1310720 (22.2%) | 23160 / 327680 (7.1%) | OLED-less baseline | 2026-09-25 |
| skeleton (M4 adapters, BT gated off) | 291873 / 1310720 (22.3%) | 23184 / 327680 (7.1%) | no Bluedroid link | 2026-09-25 |
| t-lion-debug (pre-M4 / OLED only) | 341273 / 1310720 (26.0%) | 23960 / 327680 (7.3%) | ThingPulse | 2026-09-25 |
| t-lion-debug (M4 + BTDM platform) | 1137381 / 1310720 (86.8%) | 41444 / 327680 (12.6%) | Bluedroid linked | 2026-09-25 LINKER |

**BUILD-TIME / LINKER DATA only** — not physical runtime.  
Delta for enabling Bluedroid BTDM path on T-Lion ≈ **+780 KiB flash**, **+17 KiB DRAM** vs OLED-only T-Lion build.

