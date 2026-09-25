# SN30 Pro profile research (Milestone 3)

**Status:** RESEARCH + IMPLEMENTED_UNVERIFIED parser stubs.

## Identification

| Field | Evidence | Label |
| --- | --- | --- |
| VID | `0x2DC8` (8BitDo) | DOCUMENTED (public USB-ID / Bluepad32) |
| PID | Mode/firmware dependent | Do not sole-key on one PID |
| Name | Advertising name unreliable | CLAUDE.md §56 |

## Report format

Bluepad32 and community HID dumps describe multiple Classic modes (Android / Windows / mac / …) with differing report lengths and bit maps.

BlueShift implements a **conservative** packed layout parser in `Sn30ProProfile::parseGamepad`:

- buttons lo/hi
- hat nibble
- four uint8 axes centered at 0x80

Only when `7 ≤ length ≤ 32`.

## Unsupported until physical capture

- Rumble / output reports
- Battery percent from device
- Exact mode discrimination
- Descriptor fingerprinting on our hardware

## Sources (reference — not vendored)

- Bluepad32 controller database / parsers
- Public 8BitDo HID discussions
- ESP32KeyBridge architecture (transport only)
