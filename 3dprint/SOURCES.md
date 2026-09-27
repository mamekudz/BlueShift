# Sources — BlueShift™ mechanical foundation

Retrieval date for remote checks: **2026-09-27**.

---

## Primary (already in repo)

| Source | Path / URL | Role | License / redistrib |
| --- | --- | --- | --- |
| Hardware dossier | [`docs/hardware/t-lion.md`](../docs/hardware/t-lion.md) | Electrical + pin authority | Project docs |
| Schematic extract | [`docs/hardware/evidence/t18_v2.3.txt`](../docs/hardware/evidence/t18_v2.3.txt) | Text from PDF | Derived from LilyGO PDF |
| Schematic PDF (local) | [`docs/hardware/evidence/t18_v2.3.pdf`](../docs/hardware/evidence/t18_v2.3.pdf) | Official schematic copy | LilyGO — retain attribution; do not claim ownership |
| LilyGO readme mirror | [`docs/hardware/evidence/lilygo-readme.md`](../docs/hardware/evidence/lilygo-readme.md) | Example modules | Upstream README |
| Firmware pins | [`components/hardware/t_lion/board_pins.h`](../components/hardware/t_lion/board_pins.h) | GPIO constants | Project |
| Charging GPIO note | [`docs/hardware/charging-state.md`](../docs/hardware/charging-state.md) | CHRG UNKNOWN | Project |

---

## Official remote

| Source | URL | Role | Notes |
| --- | --- | --- | --- |
| LilyGO T-Lion product | https://lilygo.cc/en-us/products/t-lion | Marketing specs incl. **size 108.51×29.01 mm** | DOCUMENTED marketing; USB-UART claim CH9102 conflicts schematic |
| GitHub TTGO-T-ControllerV2.2 | https://github.com/LilyGO/TTGO-T-ControllerV2.2 | Official examples + `t18_v2.3.pdf` | Also mirrored as TTGO-T-Lion-T18V2.2 / TTGO-T18V2.2 |
| Schematic PDF (upstream) | https://github.com/LilyGO/TTGO-T-ControllerV2.2/blob/master/t18_v2.3.pdf | Schematic | Prefer local evidence copy |
| Official `adc.ino` | https://github.com/LilyGO/TTGO-T-ControllerV2.2/blob/master/T18_V2.2/adc.ino | Button + ADC pin table | Preferred over community maps |

---

## CAD / PCB / mechanical files search (2026-09-27)

| Artifact | Result |
| --- | --- |
| STEP | **Not found** in official repo |
| Parasolid | **Not found** |
| KiCad / Eagle | **Not found** |
| Gerber / DXF | **Not found** |
| Mechanical drawing with hole table | **Not found** (only marketing size + photos) |

Therefore: no redistributed third-party CAD binaries added under `3dprint/cad/`.  
Reference geometry is **project-authored** OpenSCAD envelope (`cad/reference-bodies/`).

---

## Derived / industry references (not board-specific)

| Topic | Use | Status |
| --- | --- | --- |
| 0.96″ SSD1306 module outline / AA | Cutout candidates | DERIVED / PROVISIONAL |
| Micro-USB Type-B plug envelope | USB opening candidate | DERIVED / PROVISIONAL |
| Espressif antenna keep-out guidance | Keep-out philosophy | PROVISIONAL |
| 18650 cell envelope | Battery space | DOCUMENTED form factor |

---

## Do not copy into Git without clear redistrib rights

- Arbitrary shopping-site 3D models
- Unlicensed community STEP of T-Lion
- Photos as “dimension truth” without measurement notes

Such files may live under `3dprint/**/vendor/`, `3dprint/**/local/`, `_refs/`,
or `docs/hardware/evidence/local/` (**gitignored**).

Private **NAS backup still mirrors them** when present — see
[`docs/backup/nas-policy.md`](../docs/backup/nas-policy.md).
Backing up privately does **not** grant redistribution rights.
