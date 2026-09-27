# Enclosure specification — BlueShift™

**Status:** FOUNDATION / REFERENCE ONLY — no finished enclosure.  
**Authoritative file for mechanical detail:** this document.  
**Electrical / pin dossier:** [`docs/hardware/t-lion.md`](../docs/hardware/t-lion.md)

---

## Design intent

Simple portable shell around the LilyGO **T-Lion** board.

Required exterior openings only:

1. OLED / display
2. 5-way navigation
3. USB
4. ON/OFF (board BAT switch access **or** enclosure-mounted switch — see below)

No decorative complexity. Manual styling in Plasticity (µPhotoFrame precedent).

---

## Hardware reference

| Field | Value | Status |
| --- | --- | --- |
| Target product | LilyGO T-Lion / T-Controller | DOCUMENTED |
| Marketing / repo family | TTGO T18 **V2.2** | DOCUMENTED |
| Schematic file | `t18_v2.3.pdf` | DOCUMENTED |
| Schematic sheet titles | `power V1.0`, `ESP32 V1.0` (2018-12-22) | DOCUMENTED |
| Purchased unit revision | Unknown until unboxing | UNKNOWN |
| MCU module | ESP32-WROVER | DOCUMENTED |
| Display | 0.96″ SSD1306 128×64 I²C `0x3C` | DOCUMENTED |
| Navigation | 5-way `U28` | DOCUMENTED |
| Battery form | On-board **18650** holder `P3` | DOCUMENTED |
| Board BAT switch | `JP1` **SK-12D02** | DOCUMENTED |
| Charger | TP5400 | DOCUMENTED |
| USB | Micro-USB `J1` | DOCUMENTED |

**Revision caution:** marketing/repo “V2.2” ≠ schematic filename “v2.3”. Treat as **same product family**, revisions not silently equated. Physical silk-screen verification pending.

Machine JSON: [`dimensions/board.json`](dimensions/board.json).

---

## Coordinate system (permanent)

| Axis | Meaning |
| --- | --- |
| **+X** | right (along short board edge, width) |
| **+Y** | toward antenna / OLED end (along long edge) |
| **+Z** | top / component side (OLED facing +Z) |
| **Origin** | PCB bottom-left corner of outline, component-side face Z=0 plane at PCB top copper plane |
| **PCB plane** | Z = 0 at top of PCB laminate |
| **USB side** | −Y end (USB connector end) — **PROVISIONAL** layout assumption |
| **Antenna / OLED side** | +Y end — **PROVISIONAL** layout assumption |
| **Board insertion** | into lower shell along −Z |

Documented also in `dimensions/board.json` → `coordinateSystem`.

---

## PCB

| Dimension | Value | Status | Source |
| --- | --- | --- | --- |
| Width (X) | **29.01 mm** | DOCUMENTED | LilyGO product page “size” |
| Length (Y) | **108.51 mm** | DOCUMENTED | LilyGO product page “size” |
| Laminate thickness | ~1.6 mm typical FR4 | PROVISIONAL | Industry default — not on schematic |
| Total stack height | UNKNOWN (OLED + 18650 holder + modules) | UNKNOWN | Needs PHYSICALLY_MEASURED |
| Mounting holes | Not found in public STEP/KiCad | UNKNOWN | No official CAD in repo |
| Official STEP / KiCad / Gerber | **Not published** in TTGO-T-ControllerV2.2 | DOCUMENTED (absence) | GitHub listing 2026-09-27 |

---

## OLED

| Item | Value | Status |
| --- | --- | --- |
| Panel | 0.96″ SSD1306 | DOCUMENTED |
| Logical pixels | 128×64 | DOCUMENTED |
| Module outline | ~27.0–27.8 × ~27.0–27.8 mm typical | DERIVED / PROVISIONAL |
| Active / visible area | ~21.7 × ~10.9 mm typical for 0.96″ 128×64 | DERIVED / PROVISIONAL |
| I²C | SDA GPIO21, SCL GPIO22, addr `0x3C` | DOCUMENTED |
| On-board location | Near +Y end (with 5-way) | PROVISIONAL (photo-derived) |

### Recommended enclosure cutout

**Status: PROVISIONAL_CAD_CANDIDATE**

- Opening ≈ **active area + 0.4…0.8 mm** per side (hide module bezel / PCB edges)
- Candidate opening: **≈ 22.5 × 11.7 mm** centered on visible area
- Recess / lip: hide carrier glass border — refine after PHYSICALLY_MEASURED

Do **not** cut to full module outline if the bezel should be hidden.

---

## 5-way control

| Item | Value | Status |
| --- | --- | --- |
| Part | `U28` five-direction switch | DOCUMENTED |
| GPIOs | Up32 Down33 OK34 Left36 Right39 | DOCUMENTED (`adc.ino`) |
| Center position | Near OLED / +Y end | PROVISIONAL |
| Actuator diameter / height | UNKNOWN | PENDING physical measure |
| Travel / clearance | Must allow U/D/L/R + press without binding | PROVISIONAL |

### Direct vs printed cap

| Variant | Notes |
| --- | --- |
| **DIRECT_ACCESS** | Opening over stock actuator — preferred first prototype |
| **PRINTED_CAP** | Optional later if travel/feel insufficient — interface TBD |

Recommendation: start with **DIRECT_ACCESS**. Cap only after board in hand.

---

## USB

| Item | Value | Status |
| --- | --- | --- |
| Connector | Micro-USB SMT `J1` | DOCUMENTED |
| UART bridge | Schematic CP2104 vs marketing CH9102 | CONFLICT → UNKNOWN until measured |
| Shell size | Typical Micro-B SMT | DERIVED / PROVISIONAL |
| Cable plug clearance | Needs full plug envelope, not shell only | PROVISIONAL_CAD_CANDIDATE |

### Recommended USB cutout

**Status: PROVISIONAL_CAD_CANDIDATE**

- Opening ≥ **≈ 12 × 8 mm** (plug + wiggle) centered on connector
- Depth clearance for molded strain relief
- Refine after PHYSICALLY_MEASURED connector position

---

## ON/OFF switch

| Path | Status |
| --- | --- |
| **Board-integrated** BAT switch `JP1` SK-12D02 | DOCUMENTED / SELECTED (on PCB) |
| **Enclosure-mounted external** power switch | TO_BE_SELECTED (optional product decision) |

Do **not** invent external cutout dimensions until `PWR-01` (external) is chosen.

BOM must keep board switch vs enclosure switch distinct.

If enclosure only exposes the existing BAT switch slider: cutout = **PROVISIONAL_CAD_CANDIDATE** after measuring slider position/stroke.

---

## Battery

| Item | Value | Status |
| --- | --- | --- |
| Form factor | **18650** in holder `P3` | DOCUMENTED |
| Cell capacity / brand | — | TO_BE_SELECTED |
| Chemistry | Li-ion 18650 | DOCUMENTED form |
| Protection | Prefer **protected** cell until board BMS VERIFIED | ASSUMED policy (`t-lion.md`) |
| Charger | TP5400 ≠ complete BMS | DOCUMENTED warning |

### Safety / space

- No screw tips into cell volume
- No sharp pressure points on cell
- Cable / spring clearance in holder
- Serviceable access to replace 18650
- Battery is **not** a structural spacer

---

## Antenna keep-out

| Item | Value | Status |
| --- | --- | --- |
| Radio | ESP32-WROVER on-module PCB antenna | DOCUMENTED |
| Module location on board | Near module footprint (typically +Y) | PROVISIONAL |
| Keep-out | Avoid metal screws, dense metal, battery overlap within ~15–20 mm of antenna tip where practical | PROVISIONAL (Espressif guidance class) |

Bluetooth is the product’s primary function — do not casually fill the antenna end with metal inserts.

---

## Enclosure split

**Recommendation (not final):**

| Part | Role |
| --- | --- |
| PRINT-01 upper/front | OLED + 5-way openings, optional switch window |
| PRINT-02 lower/back | USB opening, battery retention, board seats |

Author chooses exact split plane in Plasticity.

---

## Fastening

Candidates (simple, serviceable):

- self-tapping plastic screws
- M2/M2.5 + heat-set inserts
- light snap-fit

Do not over-engineer. Must open for battery / board / switch service.

---

## Printing

Assumptions **not frozen**:

- material, layer height, wall thickness, tolerance → user selection later

Priorities for this small device:

1. clean OLED / USB / 5-way openings
2. accurate connector fit
3. serviceability

---

## Plasticity workflow

See [`plasticity/README.md`](plasticity/README.md).

---

## Known dimensions

- PCB outline 108.51 × 29.01 mm — DOCUMENTED (LilyGO size spec)
- OLED resolution / I²C / GPIOs — DOCUMENTED
- 5-way GPIO map — DOCUMENTED
- Micro-USB present — DOCUMENTED
- 18650 holder — DOCUMENTED
- BAT switch SK-12D02 — DOCUMENTED

## Unknown dimensions

- Mounting hole pattern
- Exact OLED module outline & active-area offsets on this PCB
- Exact 5-way XY / Z protrusion
- Exact USB XY / Z
- Exact antenna tip location
- PCB thickness / total height
- External switch (if any) cutout

## Pending component selection

- BAT-01 specific 18650 cell (capacity, protection, vendor URL)
- PWR-01 external enclosure switch (if required beyond board BAT switch)
- FAST-01 fastener system
- Optional PRINT-03 5-way cap
