# LILYGO T-Lion — hardware notes

**Status: UNVERIFIED** until the physical board arrives and is brought up.

BlueShift's first intended hardware target is the LILYGO **T-Lion**
(also marketed as T-Controller), a classic **ESP32-WROVER** handheld with OLED
and 18650 holder.

## Manufacturer marketing claims (not yet physically confirmed)

Sources (retail/manufacturer pages — treat as research, not VERIFIED facts):

- https://lilygo.cc/en-us/products/t-lion

Reported product features (UNVERIFIED on our unit):

| Item | Claimed | BlueShift status |
| --- | --- | --- |
| MCU | ESP32-WROVER | UNVERIFIED |
| Flash | 4 MB SPI | UNVERIFIED |
| PSRAM | 8 MB | UNVERIFIED |
| Display | 0.96″ OLED, SSD1306 | UNVERIFIED |
| Input | 5-way button + reset/boot | UNVERIFIED |
| Battery | 18650 holder, BAT switch | UNVERIFIED |
| USB-UART | CH9102 | UNVERIFIED |
| Size | ~108.51 × 29.01 mm | UNVERIFIED |

## PlatformIO board target

There is **no** official PlatformIO board ID named `lilygo-t-lion` (checked 2026-09).

Related generic boards in platform `espressif32`:

- `esp32dev`
- `esp-wrover-kit`
- `freenove_esp32_wrover`
- `upesy_wrover`

Milestone 1 uses `esp32dev` only as a **provisional compile-smoke** environment
(`skeleton` / `debug` / `release` in `platformio.ini`). That choice is **not** a
claim that T-Lion pinmux, PSRAM, flash size, or USB CDC match `esp32dev`.

## Do not hard-code yet

Until physical verification, do **not** treat the following as project facts:

- GPIO assignments
- OLED I²C pins / controller identity
- Battery ADC pin
- Button pins
- Charging status pins
- Exact flash / PSRAM sizes for the purchased revision

## Next milestone

`PHYSICAL T-LION HARDWARE BRING-UP` — separate prompt after the board arrives.
