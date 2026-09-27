# Plasticity workflow — BlueShift™ enclosure

Plasticity is the **primary manual** enclosure design tool.

Do **not** auto-generate the finished shell in Cursor.

## Import reference

1. Open Plasticity.
2. Import board reference:
   - Prefer STEP when available under `../cad/step/`
   - Until then: use OpenSCAD envelope `../cad/reference-bodies/t-lion-envelope.scad`
     (export STEP via FreeCAD/OpenSCAD when convenient), **or** build thin boxes by hand from `../dimensions/board.json`
3. Lock / layer the reference so it is not accidentally edited.

## Build sequence

```text
import board STEP / reference envelopes
        →
create enclosure around reference
        →
cut OLED opening          (PROVISIONAL_CAD_CANDIDATE → measure)
        →
cut 5-way opening         (DIRECT_ACCESS first)
        →
cut USB opening           (plug clearance, not shell-only)
        →
add ON/OFF access         (board BAT switch window and/or PWR-01B)
        →
add battery retention     (18650 holder serviceable; no screw into cell)
        →
respect antenna keep-out
        →
split enclosure           (upper/lower recommendation)
        →
add screw/snap geometry
        →
manual styling / fillets
        →
export printable parts → ../printable/
```

## Philosophy

Same as µPhotoFrame adapter work: **simple, fast, mechanically precise, manually refined**.

## Outputs expected later

| File | When |
| --- | --- |
| `../printable/print-01-upper.stl` (or 3mf) | After Plasticity |
| `../printable/print-02-lower.stl` | After Plasticity |
| optional `print-03-fiveway-cap` | Only if DIRECT_ACCESS insufficient |

## Do not

- Design the aesthetic exterior in this repo as an agent task
- Invent final cutouts without measurement
- Place metal near the ESP32 antenna tip
- Treat TP5400 as a complete BMS
