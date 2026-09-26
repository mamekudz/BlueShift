# T-Lion physical bring-up plan

**Status:** PLANNED — board not available.  
**Prerequisite:** Native ESP-IDF dual-mode compile/link proof (`docs/bluetooth/esp-idf-dual-mode-spike.md`).

Do not mark any step PHYSICALLY VERIFIED until executed on hardware.

---

## Sequence

1. Serial console / boot log (version, board flags)
2. OLED SSD1306 (I²C 0x3C, SDA21/SCL22)
3. 5-way navigation (GPIO 32/33/34/36/39)
4. Battery ADC (GPIO35, divider ×2)
5. Classic BR/EDR scan
6. SN30 Pro discovery (Classic)
7. Classic pairing / bond
8. Raw HID reports (descriptor capture)
9. BLE advertising (HOGP keyboard/gamepad)
10. BLE host connection (e.g. Windows / ESP][)
11. Classic → BLE bridge (keyboard then gamepad)
12. Reconnect (input lost / output lost / both)
13. Battery operation under bridge load
14. Long-run stability

---

## Stress / long-run (future physical)

- 100 connect/disconnect cycles
- Input reconnect only
- Output reconnect only
- Both reconnect
- Hours-long bridge operation
- Memory leak monitoring (heap watermarks)
- Battery runtime
- OLED timeout / DisplayPowerManager interaction
- Pairing reset / factory reset confirmation

Do not execute without the T-Lion.

---

## Evidence labels

| Allowed now | Forbidden until board tested |
| --- | --- |
| COMPILE / LINK VERIFIED | PHYSICALLY VERIFIED |
| HOST TESTED | PAIRING / SN30 / NIMBUS VERIFIED |
| IMPLEMENTED_UNVERIFIED | BATTERY / OLED VERIFIED |
