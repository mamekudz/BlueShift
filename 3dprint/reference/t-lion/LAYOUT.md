# T-Lion reference notes

## Spatial planning (PROVISIONAL)

Long thin board — LilyGO size **108.51 × 29.01 mm** (DOCUMENTED).

```text
        +Y  OLED / 5-way / antenna end (PROVISIONAL)
         ↑
    ┌────┴────┐
    │  OLED   │
    │  5-way  │
    │         │
    │ ESP32   │  ← module + antenna tip keep-out
    │ WROVER  │
    │         │
    │ 18650   │  ← holder P3 (DOCUMENTED form)
    │ holder  │
    │         │
    │ BAT SW  │  ← JP1 SK-12D02 (DOCUMENTED)
    │  USB    │  ← Micro-USB J1 (DOCUMENTED)
    └────┬────┘
         ↓
        -Y  USB end (PROVISIONAL)

    X → width 29.01 mm
    Z → OLED / components up
```

## Assembly checklist (planning only)

- [ ] PCB seat
- [ ] OLED opening (AA, not full module)
- [ ] 5-way DIRECT_ACCESS
- [ ] USB plug clearance
- [ ] BAT switch access or external PWR-01B
- [ ] 18650 serviceability
- [ ] Antenna keep-out free of metal clutter

## Photos

Use LilyGO product / GitHub `image/` photos for orientation only — **not** for dimension extraction without measuring.
