# OLED monochrome logo asset slot

Final artwork will be created by the project author in Illustrator.

Do **not** auto-vectorize `blueshift-logo.png` into an SVG/OLED bitmap and claim it
is the final logo.

## Planned files

| File | Role |
| --- | --- |
| `docs/assets/blueshift-oled-mono.svg` | Author-supplied mono master (future) |
| `docs/assets/blueshift-oled-mono.png` | Preview (future) |
| `src` / generated header | 128×64 (or cropped) 1-bit bitmap for SSD1306 |

## Conversion path (planned)

1. Author exports mono SVG/PNG at OLED-friendly size.
2. Gulp/Python tool converts to `XBM` / `uint8_t` page-packed SSD1306 buffer.
3. Firmware `Display` draws the bitmap on boot/about.

Temporary placeholder for layout tests: empty slot — UI uses text wordmark via i18x.
