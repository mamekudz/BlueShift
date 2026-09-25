# LILYGO T-Lion / T-Controller — hardware dossier

**Physical board status: not yet arrived. Nothing here is VERIFIED on our unit.**

Evidence classes used in this document:

| Label | Meaning |
| --- | --- |
| **DOCUMENTED** | Supported by manufacturer schematic / official LilyGO example source |
| **ASSUMED** | Reasonable working assumption; must be confirmed on hardware |
| **UNKNOWN** | Insufficient evidence |
| **VERIFIED** | *(none yet)* Physically confirmed on our T-Lion |

Primary sources:

| Source | Role |
| --- | --- |
| [LilyGO/TTGO-T-ControllerV2.2](https://github.com/LilyGO/TTGO-T-ControllerV2.2) | Official repo (also mirrored as TTGO-T-Lion-T18V2.2) |
| [`t18_v2.3.pdf`](evidence/t18_v2.3.pdf) | Schematic (local copy under `docs/hardware/evidence/`) |
| [`T18_V2.2/adc.ino`](https://github.com/LilyGO/TTGO-T-ControllerV2.2/blob/master/T18_V2.2/adc.ino) | Official pin usage example |
| [lilygo.cc T-Lion product page](https://lilygo.cc/en-us/products/t-lion) | Marketing claims (secondary) |

Local evidence extract: `evidence/t18_v2.3.txt` (text extracted from schematic PDF).

---

## Identity / revisions

| Item | Value | Confidence |
| --- | --- | --- |
| Product names | T-Lion, T-Controller, TTGO T18 V2.2 / V2.3 | DOCUMENTED |
| Schematic title dates | 2018-12-22 (`power V1.0`, `ESP32 V1.0`) | DOCUMENTED |
| Purchased board revision | Unknown until unboxing | UNKNOWN |
| PlatformIO official board ID | None named `lilygo-t-lion` | DOCUMENTED (absence) |

---

## MCU / memory

| Item | Value | Confidence |
| --- | --- | --- |
| Module | ESP32-WROVER (`U11` on schematic) | DOCUMENTED |
| SoC class | Original ESP32 (Xtensa LX6 dual-core) — BR/EDR + BLE | DOCUMENTED |
| Flash | Marketing: 4 MB SPI | ASSUMED (marketing + typical WROVER); UNKNOWN exact SKU |
| PSRAM | Marketing: 8 MB | ASSUMED; UNKNOWN exact SKU |
| Schematic also shows `ESP-WROOM-32` footprint (`U1`) | Possible alternate population | DOCUMENTED (presence of symbol) / UNKNOWN which we receive |

---

## USB / UART

| Item | Value | Confidence |
| --- | --- | --- |
| Connector | Micro-USB (`J1`) | DOCUMENTED |
| USB-UART on schematic | **CP2104** (`U10`) | DOCUMENTED |
| Marketing claim | CH9102 | DOCUMENTED conflict vs schematic — treat as **UNKNOWN for our unit** until measured |
| UART | `U0TXD` / `U0RXD` to bridge | DOCUMENTED |
| Boot / reset wiring | `IO0`, `RST/EN` via DTR/RTS auto-program | DOCUMENTED |

---

## Display (OLED)

| Item | Value | Confidence |
| --- | --- | --- |
| Panel | 0.96″ OLED, SSD1306 (`U24`) | DOCUMENTED |
| Resolution | 128×64 | DOCUMENTED (example + common SSD1306 0.96) |
| Interface | I²C (schematic resistor options for SPI/I2C; example uses I2C) | DOCUMENTED |
| Address | `0x3C` | DOCUMENTED (`adc.ino`) |
| SDA | GPIO **21** | DOCUMENTED |
| SCL | GPIO **22** | DOCUMENTED |
| Reset GPIO | Not driven in official example (internal) | ASSUMED |

---

## Navigation (5-way)

Official LilyGO `adc.ino` button table (**preferred source** over community ESPHome configs that disagree):

| Direction | GPIO | Confidence |
| --- | --- | --- |
| Up | **32** | DOCUMENTED |
| Down | **33** | DOCUMENTED |
| Center / OK / Push | **34** | DOCUMENTED |
| Left | **36** | DOCUMENTED |
| Right | **39** | DOCUMENTED |

Schematic `U28` Five direction switch: pins labeled GND / right / OK / up / left / down — DOCUMENTED.

Active level: example uses Button2 default (typically active-low with pull-up) — ASSUMED; confirm on hardware.

**Note:** GPIO 34/35/36/39 are input-only on ESP32 — DOCUMENTED (Espressif). Suitable for buttons/ADC, not outputs.

### Electrical assumptions (firmware)

| Pins | Pull-up policy | Label |
| --- | --- | --- |
| GPIO32 / GPIO33 | `INPUT_PULLUP` allowed (internal OK) | ASSUMED board also has resistors |
| GPIO34 / GPIO36 / GPIO39 | `INPUT` only — **no** internal pull-up | DOCUMENTED ESP32 limitation; board must provide resistors |
| Active level | Active-low | ASSUMED (Button2 / LilyGO example) |

Firmware: `components/input/gpio_navigation.cpp`. Do not enable unsupported internal pulls on input-only pins.

---

## LEDs

| Item | Value | Confidence |
| --- | --- | --- |
| GPIO 5 | Status / blue LED (community + schematic net `IO5`) | ASSUMED / DOCUMENTED soft |
| Charge LED(s) | Schematic shows RED / GREEN / BLUE LED footprints near charger | DOCUMENTED footprints; mapping ASSUMED |
| Charge status pin to MCU | TP5400 `CHRG` / `STDBY` — whether routed to a readable GPIO | UNKNOWN (needs schematic net trace / meter) |

---

## Battery / charger / protection

| Item | Value | Confidence |
| --- | --- | --- |
| Cell | Single 18650 holder (`P3`) | DOCUMENTED |
| BAT switch | `JP1` SK-12D02 | DOCUMENTED |
| Charger IC | **TP5400** (`U27`) | DOCUMENTED |
| Charge path | USB VBUS → TP5400 → BAT / system | DOCUMENTED |
| 3.3 V rail | SY8089 buck (`U2`) from 5 V path | DOCUMENTED |
| Battery ADC | GPIO **35**, formula in example uses `* 2.0` → **1:1 divider** (100k/100k class) | DOCUMENTED |
| Divider resistors | Schematic `R42`/`R43` 100K near BAT sense region | ASSUMED match to ADC formula |
| Fuse | `D6` labeled 2A Fuse | DOCUMENTED |
| Overcharge protection | Charger IC charge termination | DOCUMENTED (charger function) |
| Overdischarge / overcurrent BMS | Dedicated protection IC **not clearly identified** in text extract | UNKNOWN — **do not assume charger == full cell protection** |
| Protected vs unprotected cell policy | Prefer protected 18650 until board protection is VERIFIED | ASSUMED policy |

---

## Free / constrained GPIOs

| Class | Notes | Confidence |
| --- | --- | --- |
| Used onboard | 5, 21, 22, 32, 33, 34, 35, 36, 39 (+ UART0, EN, IO0) | DOCUMENTED |
| Input-only | 34, 35, 36, 39 | DOCUMENTED |
| Strapping | IO0, IO2, IO12, IO15 — care when using headers | DOCUMENTED (Espressif) |
| Header pins | Schematic shows 20-pin headers exposing many ESP32 GPIOs | DOCUMENTED |
| Safe free GPIOs for BlueShift extras | Prefer header pins not listed as onboard; verify after bring-up | ASSUMED |

---

## Bluetooth radio

| Item | Value | Confidence |
| --- | --- | --- |
| Classic BR/EDR | Present on original ESP32 module | DOCUMENTED |
| BLE | BLE 4.2 on original ESP32 | DOCUMENTED |
| Simultaneous Classic HID Host + BLE HID Peripheral | Feasible only with a dual-capable stack strategy — see `docs/bluetooth/` | ASSUMED (software) / UNKNOWN (our firmware until tested) |

---

## Conflicts / cautions

1. **USB-UART IC:** marketing CH9102 vs schematic CP2104 — verify on arrival.
2. **Community pin maps** (e.g. ESPHome issue #7) disagree with official `adc.ino` on button GPIOs — BlueShift uses **official example** until physical verification.
3. **Battery safety:** treat protection as UNKNOWN; require explicit bring-up checks before long unattended runs.

---

## BlueShift board layer mapping

Firmware must read pins only from `components/hardware/t_lion/` (DOCUMENTED constants marked `IMPLEMENTED_UNVERIFIED`). Application/UI/bridge code must not embed raw GPIO numbers.
