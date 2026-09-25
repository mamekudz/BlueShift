# Nimbus 69070 profile research (Milestone 3)

**Status:** RESEARCH + minimal IMPLEMENTED_UNVERIFIED digital stub.

## Prior physical evidence (ESP][ / ESP32-S3)

| Observation | Label |
| --- | --- |
| Not visible to ESP32-S3 BLE scanner | VERIFIED (prior project evidence) |
| Visible to Windows Bluetooth | VERIFIED (prior) |
| Implication | Classic BR/EDR candidate for BlueShift input | ASSUMED product use |

## Parser

`Nimbus69070Profile::parseGamepad` only maps first-byte digital bits A/B/X/Y when length is plausible.

Axes, hat, triggers, battery: **UNKNOWN** — not guessed.

## Next physical steps on T-Lion

1. Classic inquiry / name / COD capture
2. HID descriptor dump
3. Report capture while pressing known buttons
4. Expand profile only from captured evidence
