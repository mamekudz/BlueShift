# Wiring / power / control — BlueShift™ mechanical context

Reuse of [`docs/hardware/t-lion.md`](../docs/hardware/t-lion.md).  
Statuses: `VERIFIED` · `DOCUMENTED` · `PROPOSED` · `UNASSIGNED` · `UNKNOWN`

Do **not** invent nets or GPIOs.

---

## Power path (DOCUMENTED)

```text
USB VBUS (Micro-USB J1)
    →
TP5400 charger (U27)
    →
BAT (18650 holder P3)  ←→  BAT switch JP1 SK-12D02
    →
SY8089 buck (U2) → 3V3 rail → ESP32-WROVER / OLED / logic
```

| Element | Status |
| --- | --- |
| USB 5 V input | DOCUMENTED |
| TP5400 charge path | DOCUMENTED |
| BAT switch on battery path | DOCUMENTED |
| 3V3 from SY8089 | DOCUMENTED |
| Fuse `D6` 2A | DOCUMENTED |
| Dedicated overdischarge BMS IC | UNKNOWN — **charger ≠ complete protection** |

Policy: prefer **protected 18650** until board protection is PHYSICALLY_VERIFIED.

---

## Battery sense

| Signal | GPIO | Notes | Status |
| --- | --- | --- | --- |
| Battery ADC | **35** | Example uses ×2.0 → 1:1 divider class | DOCUMENTED |
| Divider | R42/R43 100k region | ASSUMED match to formula | DOCUMENTED soft |

Firmware: `components/hardware/t_lion/board_pins.h` — do not reassign.

---

## OLED

| Signal | GPIO / value | Status |
| --- | --- | --- |
| SDA | **21** | DOCUMENTED |
| SCL | **22** | DOCUMENTED |
| Address | **0x3C** | DOCUMENTED |
| Reset GPIO | Not in official example | ASSUMED internal |

---

## 5-way (`U28`)

| Direction | GPIO | Status |
| --- | --- | --- |
| Up | **32** | DOCUMENTED |
| Down | **33** | DOCUMENTED |
| Center / OK | **34** | DOCUMENTED |
| Left | **36** | DOCUMENTED |
| Right | **39** | DOCUMENTED |

Active-low with board pull-ups — ASSUMED; confirm on hardware.

---

## USB / UART

| Item | Status |
| --- | --- |
| Micro-USB data + power | DOCUMENTED |
| USB-UART IC | Schematic **CP2104** vs marketing **CH9102** → UNKNOWN until measured |
| Auto-program DTR/RTS → EN/IO0 | DOCUMENTED |

---

## Charging status to MCU

| CHRG / STDBY → readable GPIO | UNKNOWN |

See [`docs/hardware/charging-state.md`](../docs/hardware/charging-state.md).

---

## Enclosure-mounted switch (optional)

| External PWR-01B | UNASSIGNED |

If added later: document series interruption point (BAT vs 3V3) as **PROPOSED** only after design review — not invented here.

---

## Machine JSON

[`dimensions/wiring.json`](dimensions/wiring.json)
