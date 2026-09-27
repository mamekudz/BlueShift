# BlueShift™ — 3D print / mechanical foundation

**Purpose:** Plasticity-ready mechanical reference for a **simple** BlueShift enclosure.  
**Not included:** finished enclosure CAD, aesthetic exterior, firmware changes.

The project author models the enclosure **manually in Plasticity**.

Cursor / agents prepare:

- authoritative dimensions (with provenance)
- cutout / keep-out candidates
- BOM + wiring relationships
- board reference geometry scripts

## Layout

| Path | Role |
| --- | --- |
| [`ENCLOSURE-SPEC.md`](ENCLOSURE-SPEC.md) | **Authoritative** detailed mechanical specification |
| [`BOM.md`](BOM.md) | Parts list |
| [`WIRING.md`](WIRING.md) | Power / GPIO / connector relationships |
| [`SOURCES.md`](SOURCES.md) | Provenance of mechanical/electrical evidence |
| [`dimensions/`](dimensions/) | Machine-readable board / enclosure / wiring JSON |
| [`cad/`](cad/) | Reference bodies / STEP notes |
| [`plasticity/`](plasticity/) | Manual enclosure workflow |
| [`reference/t-lion/`](reference/t-lion/) | Board layout notes + sketches |
| [`printable/`](printable/) | Future printable exports (empty until authored) |
| [`prototypes/`](prototypes/) | Future print trials |

## Philosophy (µPhotoFrame-style)

Simple · fast · mechanically precise · manually refined.

Do **not** invent a heavy parametric framework. Do **not** auto-generate the finished enclosure.

## Status vocabulary

Mechanical facts: `OFFICIAL` · `DOCUMENTED` · `PHYSICALLY_MEASURED` · `DERIVED` · `PROVISIONAL` · `UNKNOWN` · `PENDING_COMPONENT_SELECTION`

BOM: `SELECTED` · `ORDERED` · `RECEIVED` · `PHYSICALLY_VERIFIED` · `CANDIDATE` · `TO_BE_SELECTED`

Hardware electrical dossier remains: [`docs/hardware/t-lion.md`](../docs/hardware/t-lion.md).
