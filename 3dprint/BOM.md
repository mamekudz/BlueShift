# BOM — BlueShift™ enclosure / mechanical

Statuses: `SELECTED` · `ORDERED` · `RECEIVED` · `PHYSICALLY_VERIFIED` · `CANDIDATE` · `TO_BE_SELECTED`

Do not invent shopping links for unknown parts.

| ID | Component | Function | Qty | Exact model | Dimensions | Electrical | Mounting | Status | CAD dependency | Supplier | Product URL | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| MAIN-01 | T-Lion board | MCU + OLED + 5-way + charger + 18650 holder | 1 | LilyGO T-Lion / T-Controller (TTGO T18 V2.2 family; schematic `t18_v2.3`) | PCB 108.51×29.01 mm (DOCUMENTED) | ESP32-WROVER, Micro-USB, TP5400 | Seat in enclosure | SELECTED (target HW) | Yes — reference body | LilyGO | https://lilygo.cc/en-us/products/t-lion | Purchased unit rev UNKNOWN until unbox |
| BAT-01 | 18650 Li-ion cell | Portable energy | 1 | — | Form: 18650 (~18×65 mm) | Prefer protected cell | In board holder `P3` | TO_BE_SELECTED | Envelope only | — | | TP5400 ≠ full BMS — see `docs/hardware/t-lion.md` |
| PWR-01A | Board BAT switch | Battery disconnect on PCB | 1 | SK-12D02 (`JP1`) | On-board | Series BAT path | PCB | SELECTED (on MAIN-01) | Cutout if exposed | — | | Board-integrated |
| PWR-01B | Enclosure ON/OFF | Optional external power control | 0–1 | — | — | TBD | Enclosure | TO_BE_SELECTED | Cutout TBD | — | | Distinct from PWR-01A; do not invent cutout |
| FAST-01 | Enclosure fasteners | Shell join | TBD | — | — | n/a | Corners / bosses | TO_BE_SELECTED | Boss geometry | — | | Prefer simple serviceable screws |
| PRINT-01 | Upper / front shell | OLED + 5-way (+ optional switch window) | 1 | BlueShift print | TBD Plasticity | n/a | Fasten to PRINT-02 | TO_BE_SELECTED | Yes | — | | Author models in Plasticity |
| PRINT-02 | Lower / back shell | USB + battery retention + board seat | 1 | BlueShift print | TBD Plasticity | n/a | Fasten to PRINT-01 | TO_BE_SELECTED | Yes | — | | Author models in Plasticity |
| PRINT-03 | Optional 5-way cap | Soft actuator | 0–1 | — | Interface TBD | n/a | Over `U28` | CANDIDATE | Optional | — | | Prefer DIRECT_ACCESS first |
