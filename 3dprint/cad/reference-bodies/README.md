# CAD reference bodies

## Official CAD

As of **2026-09-27**, LilyGO’s TTGO-T-ControllerV2.2 repository publishes:

- schematic PDF (`t18_v2.3.pdf`)
- Arduino examples / images

It does **not** publish STEP, Parasolid, KiCad, Eagle, Gerber, or DXF mechanical drawings.

Therefore this folder holds **project-authored reference envelopes only**.

## Files

| File | Format | Role |
| --- | --- | --- |
| `t-lion-envelope.scad` | OpenSCAD | PCB outline (DOCUMENTED size) + PROVISIONAL feature zones |
| `../step/README.md` | notes | Where STEP exports should land |
| `../parasolid/README.md` | notes | Optional Parasolid exports |

## Export to Plasticity

1. Open `t-lion-envelope.scad` in OpenSCAD or FreeCAD.
2. Export **STEP** (preferred) or STL.
3. Place STEP under `../step/board-reference.step`.
4. Import into Plasticity per `../../plasticity/README.md`.

Do **not** treat envelope feature positions as PHYSICALLY_MEASURED.
