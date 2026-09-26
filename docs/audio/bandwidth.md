# Speaker-edge bandwidth analysis (Apple II → BLE)

**Status:** ANALYTICAL / ESTIMATED — not physical measurement.  
Clock reference: `kApple2CyclesPerSecond = 1_020_484` (NTSC community DOCUMENTED).

---

## Toggle-rate scenarios

| Scenario | Edges/s (order) | Notes |
| --- | --- | --- |
| Quiet / rare beeps | << 100 | trivial |
| Game SFX / square-ish tones | ~1k–10k | typical |
| Fast square (~5 kHz audio) | ~10k | 2 edges/period |
| Digitized / PWM-like | ~20k–40k | demanding |
| Pathological: toggle every CPU cycle | ~1.02e6 | **not transportable** |

Pathological max is a theoretical upper bound, not a product requirement.

---

## Wire cost (v1 EdgeBatch = format C)

Header 10 B + 2 B × edges.

| Edges/frame | Frame bytes | @ 10k edges/s | @ 40k edges/s |
| --- | --- | --- | --- |
| 16 | 42 | ~26 KB/s | ~105 KB/s |
| 32 | 74 | ~23 KB/s | ~93 KB/s |
| 64 | 138 | ~22 KB/s | ~86 KB/s |

Batching amortizes header overhead.

---

## BLE GATT throughput (realistic)

| Path | Rough ceiling | Label |
| --- | --- | --- |
| Default ATT MTU 23 | low tens of KB/s under contention | ESTIMATED |
| DLE / larger MTU | higher tens–low hundreds KB/s | ESTIMATED |
| Shared with HID notifications | reduced audio budget | ASSUMED |

**Conclusion:** Ordinary beeps and many game sounds are plausible.  
Aggressive digitized/PWM streams need MTU negotiation, batching, and possibly rate limiting / quality modes — **do not assume automatic fit**.

---

## Comparison notes

- Format B (varint) helps sparse streams; CPU cost higher.  
- Format D helps resync; larger headers.  
- V1 default remains batched u16 (C).

---

## Product implication

Measure ESP][ → BlueShift edge rate on hardware before marketing digitized audio quality.
