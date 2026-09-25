# Third-party licenses

BlueShift tracks researched vs. actually incorporated dependencies separately.

## Incorporated into this repository / build

| Project | Version | License | Source | Modifications |
| --- | --- | --- | --- | --- |
| ThingPulse esp8266-oled-ssd1306 | 4.6.2 | MIT | https://github.com/ThingPulse/esp8266-oled-ssd1306 | None — PlatformIO `lib_deps` on `t-lion-*` only |
| PlatformIO `espressif32` | 6.9.0 | Apache-2.0 (platform) | PlatformIO registry | Build toolchain pin |

Build tooling (not firmware source):

| Tool | Pin | Notes |
| --- | --- | --- |
| npm `gulp` | ^5.0.0 | `package.json` |
| npm `gulp-mu-gulp-api` | ^0.5.0 | µGulp task API |

## Evaluated / referenced (NOT default-linked)

| Project | Role | License note | Source |
| --- | --- | --- | --- |
| NimBLE-Arduino (`h2zero`) | Optional BLE peripheral spike | Apache-2.0 | https://github.com/h2zero/NimBLE-Arduino — enable via `BLUESHIFT_ENABLE_NIMBLE` |
| Bluepad32 | Classic gamepad host research | Verify before adopt | https://github.com/ricardoquesada/bluepad32 |
| BTstack | Bluepad32 underlying stack | Verify before adopt | https://github.com/bluekitchen/btstack |
| ESP32KeyBridge | Classic→BLE bridge architecture reference | MIT | https://github.com/tanakamasayuki/ESP32KeyBridge |
| EspBle / EspBleClassic | Dual-host candidate (BLE + Classic) | MIT + bundled notices | https://github.com/tanakamasayuki/EspBle |
| Button2 | Button helper (LilyGO example) | Verify before adopt | https://github.com/lewisxhe/Button2 |
| LilyGO TTGO-T-ControllerV2.2 | Schematic + example pins | Manufacturer repo | https://github.com/LilyGO/TTGO-T-ControllerV2.2 |

See `docs/bluetooth/m3-stack-decision.md` — dual Classic+BLE radio **not** linked on Arduino 2.x / espressif32 6.9.0 to avoid stack duplication.

## Assets

| Asset | Origin | Notes |
| --- | --- | --- |
| `docs/assets/microgulp-ready.png` | Official µGulp ready artwork | Unchanged |
| `docs/assets/blueshift-logo.png` | BlueShift draft logo | Full-color; OLED mono TBD by author |
| `components/ui/oled_logo_placeholder.h` | Temporary 1-bit slot | **Not** final logo |
| `docs/hardware/evidence/t18_v2.3.pdf` | LilyGO schematic copy | Evidence only |
