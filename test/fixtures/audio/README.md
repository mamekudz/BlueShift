# Audio protocol fixtures (neutral)

**Status:** HOST-TESTABLE shared contract  
**Physical:** UNVERIFIED  

Neutral fixtures + **golden wire bytes** (`golden_*.json`, `GOLDEN.md`) are the
cross-project interoperability contract. ESP][ must not need BlueShift C++ to
validate fixtures.

Origin model (documented, not copied):

- Apple II speaker toggles on each edge
- Cycle clock: `1020484` Hz (NTSC community reference)
- ESP][ exposes `SpeakerEvent { cycle, level }` and delta-encodes gaps

Format: JSON

```json
{
  "name": "square_1k_approx",
  "apple2CyclesPerSecond": 1020484,
  "startLevel": 0,
  "edges": [ { "deltaCycles": 510 }, ... ],
  "notes": "..."
}
```

`deltaCycles` is the gap before each toggle (same semantics as protocol EdgeBatch deltas).

## Included scenarios

| File | Purpose |
| --- | --- |
| `square_wave.json` | simple square |
| `short_pulse.json` | short pulse |
| `subsample_pulse.json` | pulse shorter than one PCM sample @ 44.1 kHz |
| `pwm_varying.json` | varying PWM widths |
| `silence_gap.json` | silence / long gap (escape encoding) |
| `resync.json` | sync marker mid-stream |
| `packet_loss.json` | sequence gap scenario |

## Star Blazer

**Star Blazer = SUBJECTIVE_REFERENCE** for future physical/end-to-end listening tests.

Do **not** include or download Star Blazer media in this repository.
