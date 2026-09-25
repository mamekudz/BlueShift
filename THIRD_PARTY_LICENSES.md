# Third-party licenses

BlueShift tracks **incorporated / distributed** components separately from
**evaluated / reference-only** projects.

Authoritative inventory: [`docs/licensing/license-audit.md`](docs/licensing/license-audit.md).  
Policy intent (not legal text): [`docs/licensing/license-policy-draft.md`](docs/licensing/license-policy-draft.md).

BlueShift’s future project license will cover **original BlueShift material
only**. It does **not** re-license third-party code.

Baseline implementation commit for this update: `b9d3506`.

---

## Incorporated / distributed with firmware builds

| Project | Version / pin | License | Source | How used | Modifications |
| --- | --- | --- | --- | --- | --- |
| ThingPulse ESP8266/ESP32 OLED SSD1306 | **4.6.2** | **MIT** | https://github.com/ThingPulse/esp8266-oled-ssd1306 | PlatformIO `lib_deps` on `t-lion-debug` / `t-lion-release`; wrapped by BlueShift `Ssd1306Display` | None (not vendored) |
| PlatformIO platform `espressif32` | **6.9.0** | Apache-2.0 (platform) + package notices | PlatformIO registry | Build platform | None |
| Arduino-ESP32 framework (via platform) | as shipped by platform 6.9.0 | Espressif **Apache-2.0** family; see package NOTICE | https://github.com/espressif/arduino-esp32 | Firmware framework (`Wire`, `Preferences`, HAL, …) | None |
| ESP-IDF components (via Arduino) | as pulled by framework | Primarily **Apache-2.0** + nested 3rd-party notices | Espressif | Linked when used by framework features | None |

When distributing firmware binaries, retain applicable MIT/Apache notices
(at least ThingPulse MIT copyright and Espressif notices from the build tree).

### Development-only (not in firmware image)

| Tool | Pin | License | Notes |
| --- | --- | --- | --- |
| npm `gulp` | ^5.0.0 | MIT | Docs / format / backup tasks |
| npm `gulp-mu-gulp-api` | ^0.5.0 | MIT | µGulp task API (`https://microgulp.dev`) |
| Node.js `node:test` | host Node ≥18 | Node license | Host unit tests |

---

## Evaluated / reference only — NOT dependencies

These projects informed research or docs. They are **not** in `lib_deps`,
**not** vendored under `vendor/` / `lib/`, and **not** linked by default builds.

| Project | Role | License (verified upstream) | Source | BlueShift status |
| --- | --- | --- | --- | --- |
| Bluepad32 | Classic gamepad host research | **Apache-2.0** (+ BTstack dependency notice) | https://github.com/ricardoquesada/bluepad32 | REFERENCE_ONLY |
| BTstack (BlueKitchen) | Stack under Bluepad32 | Modified BSD-style with **non-commercial clause**; commercial license from BlueKitchen | https://github.com/bluekitchen/btstack · LICENSE fetched 2026-09-25 | REFERENCE_ONLY — **commercial use requires BlueKitchen deal** |
| ESP32KeyBridge | Classic→BLE architecture reference | **MIT** | https://github.com/tanakamasayuki/ESP32KeyBridge | REFERENCE_ONLY — **no code copied** |
| EspBle / EspBleClassic | Dual-host candidate | **MIT** | https://github.com/tanakamasayuki/EspBle | REFERENCE_ONLY — **no code copied** |
| NimBLE-Arduino (`h2zero`) | Optional BLE spike (flag off) | **Apache-2.0** (+ bundled notices) | https://github.com/h2zero/NimBLE-Arduino | Documented optional; **not default-linked** |
| Button2 | LilyGO button example mention | Verify before adopt | https://github.com/lewisxhe/Button2 | REFERENCE_ONLY |
| LilyGO TTGO-T-ControllerV2.2 | Schematic + pin evidence | Manufacturer materials | https://github.com/LilyGO/TTGO-T-ControllerV2.2 | Evidence under `docs/hardware/evidence/` |

See also `docs/bluetooth/m3-stack-decision.md`.

---

## Assets

| Asset | Origin | License / rights note |
| --- | --- | --- |
| `docs/assets/microgulp-ready.png` | Official µGulp Ready artwork | Badge use per https://microgulp.dev/de/ready/ — **not** a firmware license |
| `docs/assets/blueshift-logo.png` | BlueShift draft logo (author) | Word mark **BlueShift™**; figurative logo mark **UNDECIDED** — see `docs/licensing/branding-policy.md` |
| `components/ui/oled_logo_placeholder.h` | Temporary 1-bit placeholder | BlueShift; not final logo |
| `docs/hardware/evidence/t18_v2.3.pdf` (+ `.txt`) | LilyGO schematic evidence copy | Manufacturer copyright; documentation evidence only |

---

## Original BlueShift material

Firmware under `src/`, `include/`, `components/` (except calls into ThingPulse),
tests, most docs, and tooling are **original BlueShift work**. Final project
license is **being prepared** — see README Licensing section and
`docs/licensing/license-policy-draft.md`.
