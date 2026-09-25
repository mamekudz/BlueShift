# Third-party licenses

BlueShift tracks researched vs. actually incorporated dependencies separately.

## Incorporated into this repository

| Project | Version | License | Source | Modifications |
| --- | --- | --- | --- | --- |
| *(none yet — milestone 1 foundation only)* | — | — | — | — |

Toolchain / build dependencies used to develop BlueShift (not shipped as firmware
source) will be listed here when pinned into the project (PlatformIO packages,
npm packages such as `gulp` / `gulp-mu-gulp-api`).

## Evaluated / referenced (NOT incorporated)

These projects were researched for architecture or compatibility. Their code is
**not** copied into BlueShift at this milestone.

| Project | Role | License note | Source |
| --- | --- | --- | --- |
| Bluepad32 | Classic/BLE gamepad stack research | Verify before adopt | https://github.com/ricardoquesada/bluepad32 |
| BTstack | Possible Classic BT stack research | Verify before adopt | https://github.com/bluekitchen/btstack |
| ESP32KeyBridge | Related Classic→BLE bridge reference | Verify before adopt | https://github.com/tanakamasayuki/ESP32KeyBridge |
| PlatformIO `espressif32` | Planned firmware platform (pinned in `platformio.ini`) | PlatformIO / Espressif terms | https://github.com/platformio/platform-espressif32 |
| µGulp / `gulp-mu-gulp-api` | Task runner API (npm) | See package license | https://microgulp.dev/ |

Before adopting Bluepad32 or BTstack into firmware, verify the exact licenses
applicable to this open-source project and record them in the **Incorporated**
table (see `CLAUDE.md` §78–§79).

## Assets

| Asset | Origin | Notes |
| --- | --- | --- |
| `docs/assets/microgulp-ready.png` | Official µGulp ready artwork | Unchanged canonical badge |
| `docs/assets/blueshift-logo.png` | BlueShift logo concept (project draft) | Full-color; OLED mono variant TBD |
