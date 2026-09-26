# Audio architecture — BlueShift™ ESP][ speaker → A2DP

**Status:** RESEARCH / EXPERIMENTAL / HOST-TESTABLE layers  
**Physical:** nothing PHYSICALLY VERIFIED (T-Lion pending).  
**V1 promise unchanged:** Classic HID → BLE HID remains primary.

---

## Motivation

BlueShift may optionally expand from a controller bridge into a **Bluetooth-generation bridge**.

Primary (V1):

```text
Classic HID device → BlueShift → BLE HID → modern host / ESP][
```

Optional audio (experimental):

```text
ESP][ Apple II speaker edges
        → BLE Extension Service
        → BlueShift edge→PCM
        → A2DP Source
        → Classic BT headphones/speaker
```

First use case is **not** generic music streaming. It is cycle-accurate Apple II speaker reconstruction.

---

## Apple II speaker requirement

Speaker output must **not** be reduced to beep/note/frequency.

Apple II software toggles the speaker with precise timing (including PWM/digitized audio).  
ESP][ preserves cycle-accurate edges; BlueShift must reconstruct PCM from that timeline.

Preferred source representation:

- `deltaCycles` between toggles (state bit optional because `$C030` toggles)

Wire format is versioned; see [`docs/protocol/esp2-extension.md`](../protocol/esp2-extension.md).

---

## Pipeline (independently testable)

```text
BLE Extension Receiver  (enqueue only — no SBC/OLED)
        ↓
Edge / Jitter Buffer    (bounded; Apple II timeline)
        ↓
Edge→PCM Renderer       (integrate across sample interval)
        ↓
PCM mono s16            (44.1 / 48 kHz)
        ↓
A2DP Source pull-cb     (ESP-IDF Bluedroid)
```

**Do not** reproduce BLE arrival timing as audio timing.

---

## Triple-role Bluetooth (feasibility)

Target: **one** original-ESP32 binary, **one** Bluedroid BTDM stack:

| Role | Profile |
| --- | --- |
| Classic HID Host | input device |
| BLE HID Device | host / ESP][ |
| A2DP Source | headphones |

Spikes:

| Env | Purpose |
| --- | --- |
| `t-lion-idf-spike` | dual HID regression |
| `t-lion-idf-spike-a2dp` | A2DP Source only |
| `t-lion-idf-spike-triple` | all three LINKED |

BTstack / NimBLE: **not used**.

---

## Failure isolation

| Failure | HID bridge |
| --- | --- |
| A2DP connect fail | continues |
| Audio underrun | continues (silence + resync) |
| Edge overflow | continues (bounded drop + metric) |

`ConnectionTriplet` tracks `inputConnected` / `hostConnected` / `audioConnected` independently.

---

## Latency (provisional)

Jitter buffer protects against BLE burstiness; target is **responsive game audio**, not studio monitoring.

**Do not claim a final latency** before physical measurement.

HID forwarding remains scheduling priority over audio encode.

---

## Future / RESEARCH (not implemented)

Generic audio inputs (BLE PCM, USB, I2S) → A2DP — documented only.  
Do **not** market “Bluetooth audio bridge” as finished.

---

## Related

- [`docs/protocol/esp2-extension.md`](../protocol/esp2-extension.md)
- [`docs/audio/a2dp-codec-path.md`](a2dp-codec-path.md)
- [`docs/audio/generic-audio-future.md`](generic-audio-future.md)
- [`docs/audio/bandwidth.md`](bandwidth.md)
- [`docs/audio/a2dp-triple-role-spike.md`](a2dp-triple-role-spike.md)
- [`docs/audio/licensing-a2dp.md`](licensing-a2dp.md)
- [`docs/audio/role-conflicts.md`](role-conflicts.md)
- [`test/fixtures/audio/README.md`](../../test/fixtures/audio/README.md)
