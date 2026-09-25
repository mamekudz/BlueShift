# Power budget (provisional)

**Labels:** DOCUMENTED / ESTIMATED only. Do not advertise runtime as fact.

## Cell

| Item | Value | Label |
| --- | --- | --- |
| Chemistry | Li-ion 18650 | DOCUMENTED / product intent |
| Capacity (nominal) | ~2500 mAh | ASSUMED product target |
| Charger IC | TP5400 | DOCUMENTED (schematic) |

## Current draws (ESTIMATED — replace with DMM readings)

| Mode | Current | Confidence |
| --- | --- | --- |
| ESP32 active, radios off, OLED on | 40–80 mA | ESTIMATED |
| + Classic BT connected | +30–80 mA | ESTIMATED |
| + BLE connected | +10–40 mA | ESTIMATED |
| OLED off / dim | −5–15 mA vs full | ESTIMATED |
| Deep sleep | low mA / µA | ESTIMATED; policy not enabled yet |
| Pairing (both scanning/adv) | high end of above | ESTIMATED |

## Runtime estimator (formula only)

```
runtime_hours ≈ (capacity_mAh * usable_fraction) / average_current_mA
```

Example placeholders (NOT product claims):

- usable_fraction ≈ 0.85 (ESTIMATED)
- average_current_mA = measured once hardware arrives

## OLED power policy (software)

| Mode | Behavior |
| --- | --- |
| ACTIVE | full contrast after nav / connection change |
| DIM | after half of `oledTimeoutSec` |
| OFF | after `oledTimeoutSec` |

See `components/display/oled_power.h`.
