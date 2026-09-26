# BlueShift Extension Protocol (ESP][ boundary)

**Status:** DESIGNED / HOST-TESTABLE codec  
**Physical exchange with ESP][:** UNVERIFIED  
**Scope:** BLE vendor service framing only — no emulator code in BlueShift, no Classic/A2DP in ESP][.

---

## Service concept

Working name: **BlueShift Extension Service**

Optional GATT service on the **same BLE connection** as standard HID where practical.

Hosts that ignore the service still see a normal BLE HID device.

**Do not** stuff audio timing into HID reports.

---

## UUID strategy

Custom 128-bit vendor UUIDs (not Bluetooth SIG 16-bit allocations).

Mnemonic base (documented intent): BlueShift vendor block  
`6b6c7565-5368-6966-74xx-xxxxxxxxxxxx` (“blueShift” packing).

| Object | Offset intent |
| --- | --- |
| Service | `…0001` |
| Capabilities (read) | `…0002` |
| Control (write) | `…0003` |
| Edge stream (write / notify) | `…0004` |

Exact registration constants live in `components/protocol/extension_protocol.h` — allocate carefully before production advertising.

---

## Versioning

| Field | Value |
| --- | --- |
| Protocol version | **1** |
| Incompatible versions | reject with `UnsupportedVersion` |

---

## Capabilities (v1)

| Bit | Name | Advertise when |
| --- | --- | --- |
| 0 | `SPEAKER_EDGE_STREAM` | edge pipeline implemented |
| 1+ | haptics/status | **not** advertised until real |

Handshake:

1. Host discovers Extension Service  
2. Host reads Caps (version + capability mask)  
3. Host sends `START_AUDIO`  
4. Host writes EdgeBatch frames  

Do not infer mode from BLE device name alone.

---

## Stream commands

| Command | Meaning |
| --- | --- |
| `START_AUDIO` | begin accepting edges |
| `STOP_AUDIO` | stop; renderer may flush to silence |
| `SYNC_AUDIO` | timeline reset / marker |
| `RESET_AUDIO` | clear buffers + metrics |

---

## Packet formats researched

| ID | Format | Notes |
| --- | --- | --- |
| A | fixed `uint16` deltaCycles | simple; wasteful for sparse beeps |
| B | varint deltas | denser; more CPU |
| **C (v1 default)** | batched fixed `uint16` deltas | good MTU fit |
| D | timestamp + edge batch | heavier; useful for resync |

V1 wire: **C** — `EdgeBatch` header + N×`uint16` LE deltas.

Header (10 bytes):

| Field | Size |
| --- | --- |
| type | 1 (`0x10`) |
| encoding | 1 (`0` = fixed u16) |
| sequence | 2 |
| baseCycle | 4 |
| edgeCount | 2 |

Max edges/frame (v1): **64** → ~138 bytes payload (fits common DLE MTUs; may fragment at default ATT MTU 23).

---

## Sequence / resync

- Monotonic `sequence` (wraps at 16-bit)  
- Gap → increment diagnostic; accept or request `SYNC_AUDIO`  
- `SyncMarker` carries `timelineCycle` + reason  
- Underrun → silence + drop stale edges (no late replay)

---

## Callback safety

BLE write callback: decode + enqueue only.  
No SBC encode, no heavy PCM, no OLED, no NVS.

---

## Cross-project boundary

| In BlueShift | In ESP][ |
| --- | --- |
| Extension GATT server (optional) | Extension GATT client |
| Edge→PCM + A2DP | Cycle-accurate speaker edges |
| Classic HID Host | (unchanged) |

Shared contract: **this protocol document** + `components/protocol/*`.
