# T-Lion charging state visibility

**Question:** Does the documented T-Lion expose a reliable charging-status GPIO to the ESP32?

## Evidence reviewed

| Source | Finding |
| --- | --- |
| LilyGO schematic `t18_v2.3.pdf` | TP5400 charger present |
| LilyGO `adc.ino` / examples | Battery ADC on GPIO35; no CHRG GPIO sample |
| Marketing / wiki | Charge LED mentioned |

## Conclusion

| Signal | Status |
| --- | --- |
| Battery voltage via ADC GPIO35 | DOCUMENTED |
| Divider ×2 | DOCUMENTED |
| MCU-readable CHRG / STDBY pin | **UNKNOWN** — not confirmed in examples |
| Charge LED | May exist without MCU visibility |

## Firmware policy

`BatterySample.charging` / `externalPower` remain **false / unknown** until a verified GPIO mapping exists.

Do **not** invent a charging pin. Prefer reporting UNKNOWN over false OK.
