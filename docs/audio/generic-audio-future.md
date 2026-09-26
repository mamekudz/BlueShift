# Generic audio bridging (FUTURE / RESEARCH)

**Status:** DOCUMENTED ONLY — **not implemented** in this milestone.

BlueShift V1 audio is specifically:

```text
ESP][ speaker edges → Extension Service → PCM → Classic A2DP Source
```

Possible future inputs (not advertised as product features yet):

- generic BLE audio transport
- USB audio
- I2S input

Possible future output remains Classic A2DP (already spiked).

Do **not** implement generic arbitrary audio bridging until the ESP][ speaker path is physically proven and HID latency remains acceptable.
