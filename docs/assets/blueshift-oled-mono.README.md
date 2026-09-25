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

## Conversion path

1. Author exports mono SVG → PNG or binary **PBM P4** at OLED-friendly size (e.g. 32×16 or 64×32).
2. Run `node dev/tools/oled-mono-convert.mjs <file.pbm>` → `components/ui/oled_logo_generated.h`.
3. Firmware draws via `Display::drawBitmap` (About/boot).

Temporary placeholder: `components/ui/oled_logo_placeholder.h` — **not** the final logo.
