# Third-party licenses

BlueShift tracks researched vs. actually incorporated dependencies separately.

## Incorporated into this repository

| Project | Version | License | Source | Modifications |
| --- | --- | --- | --- | --- |
| *(none — no Bluepad32 / EspBle / BTstack / OLED driver vendored yet)* | — | — | — | — |

Build toolchain pins (not firmware source):

| Tool | Pin | Notes |
| --- | --- | --- |
| PlatformIO `espressif32` | 6.9.0 | `platformio.ini` |
| npm `gulp` | ^5.0.0 | `package.json` |
| npm `gulp-mu-gulp-api` | ^0.5.0 | µGulp task API |

## Evaluated / referenced (NOT incorporated)

| Project | Role | License note | Source |
| --- | --- | --- | --- |
| Bluepad32 | Classic gamepad host research | Verify before adopt | https://github.com/ricardoquesada/bluepad32 |
| BTstack | Bluepad32 underlying stack | Verify before adopt | https://github.com/bluekitchen/btstack |
| ESP32KeyBridge | Classic→BLE bridge architecture reference | MIT | https://github.com/tanakamasayuki/ESP32KeyBridge |
| EspBle / EspBleClassic | Dual-host candidate (BLE + Classic) | MIT + bundled notices | https://github.com/tanakamasayuki/EspBle |
| ThingPulse esp8266-oled-ssd1306 | OLED driver candidate (LilyGO example) | Verify before adopt | https://github.com/ThingPulse/esp8266-oled-ssd1306 |
| Button2 | Button helper (LilyGO example) | Verify before adopt | https://github.com/lewisxhe/Button2 |
| LilyGO TTGO-T-ControllerV2.2 | Schematic + example pins | Manufacturer repo | https://github.com/LilyGO/TTGO-T-ControllerV2.2 |

See also `docs/bluetooth/*.md`.

## Assets

| Asset | Origin | Notes |
| --- | --- | --- |
| `docs/assets/microgulp-ready.png` | Official µGulp ready artwork | Unchanged |
| `docs/assets/blueshift-logo.png` | BlueShift draft logo | Full-color; OLED mono TBD by author |
| `docs/hardware/evidence/t18_v2.3.pdf` | LilyGO schematic copy | Evidence only |
