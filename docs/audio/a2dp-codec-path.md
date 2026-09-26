# A2DP PCM → SBC path (ESP-IDF Bluedroid)

**Status:** DOCUMENTED from ESP-IDF 5.3.1 / spike evidence  
**Physical playback:** UNVERIFIED

## Where SBC runs

BlueShift supplies **signed 16-bit mono PCM** (44.1 kHz and/or 48 kHz supported at the renderer).

ESP-IDF Bluedroid **A2DP Source** pulls PCM via `esp_a2d_source_register_data_callback` and performs **SBC encoding inside the stack** (`external/sbc/encoder`, Apache-2.0).

BlueShift must **not** add another SBC/AAC library unless a future SEP requires it (separate license audit).

```text
Edge→PCM renderer  →  s16 mono ring  →  A2DP data_cb (pull)  →  Bluedroid SBC  →  Classic RFCOMM/L2CAP
```

## Callback rules

| Context | Allowed |
| --- | --- |
| BLE extension write | validate + enqueue edges only |
| Audio worker | jitter drain + PCM render + supply ring |
| A2DP data_cb | pull prepared PCM (or silence on underrun) |

Do not parse BLE packets or integrate edges inside the A2DP callback.

## Underrun

Output silence / neutral PCM; increment diagnostic; **preserve HID**; do not replay old audio later.
