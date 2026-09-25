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

Populate after CI/local `pio run -e t-lion-debug`:

| Env | Flash used | DRAM used | Notes | Measured |
| --- | --- | --- | --- | --- |
| skeleton | 290569 / 1310720 (22.2%) | 23160 / 327680 (7.1%) | no OLED lib | 2026-09-25 local |
| t-lion-debug | 341273 / 1310720 (26.0%) | 23960 / 327680 (7.3%) | ThingPulse 4.6.2 linked | 2026-09-25 local |

Delta OLED library ≈ +50 KiB flash / +0.8 KiB DRAM (order of magnitude from size report).

