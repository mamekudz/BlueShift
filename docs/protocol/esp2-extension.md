# BlueShift Extension Protocol (ESP][ boundary)

**Status:** SPECIFIED / HOST-TESTABLE  
**Physical exchange with ESP][:** UNVERIFIED  
**Scope:** BLE vendor service framing only — no emulator code in BlueShift, no Classic/A2DP in ESP][.

---

## Service concept

Working name: **BlueShift Extension Service**

Optional GATT service on the **same BLE connection** as standard HID where practical.

Hosts that ignore the service still see a normal BLE HID device.

**Do not** stuff audio timing into HID reports.

Mode is established only when the host:

1. discovers the Extension Service;
2. reads protocol version + capabilities;
3. explicitly enables the speaker stream (`START`).

Do **not** detect ESP][ by BLE device name alone.

---

## UUID strategy (STABLE — FROZEN for Protocol V1)

Custom 128-bit vendor UUIDs (not Bluetooth SIG 16-bit allocations).

Mnemonic base: `6b6c7565-5368-6966-74NN-000000000000` (“blue” + “Shif” + “t” packing).

| Object | UUID string | NN | Status |
| --- | --- | --- | --- |
| Service | `6b6c7565-5368-6966-7401-000000000000` | 01 | **STABLE** |
| Capabilities (read) | `6b6c7565-5368-6966-7402-000000000000` | 02 | **STABLE** |
| Control (write) | `6b6c7565-5368-6966-7403-000000000000` | 03 | **STABLE** |
| Edge stream (write from host) | `6b6c7565-5368-6966-7404-000000000000` | 04 | **STABLE** |

**Single source of truth:** `components/protocol/extension_protocol.h`  
(`kServiceUuid128` / `k*UuidString`). Do not scatter duplicate UUID literals.

Do not change these casually after this freeze.

---

## Versioning

| Field | Value |
| --- | --- |
| Protocol version | **1** |
| Incompatible versions | reject with `UnsupportedVersion` |

Future versions are **not** assumed packet-compatible. Use an explicit version field in Caps.

---

## Capabilities (v1)

| Bit | Name | Advertise when |
| --- | --- | --- |
| 0 | `SPEAKER_EDGE_STREAM` | edge pipeline implemented |
| 1 | `HAPTICS` | reserved — **not** advertised |
| 2 | `STATUS` | reserved — **not** advertised |

Only advertise capabilities that are actually implemented.

---

## Stream commands (minimal)

| Command | Code | Meaning |
| --- | --- | --- |
| `NOP` | 0 | no-op |
| `START` | 1 | begin accepting edges |
| `STOP` | 2 | stop; renderer may flush to silence |
| `SYNC` | 3 | soft timeline sync (prefer SyncMarker frame) |
| `RESET` | 4 | clear buffers + metrics |

Not a generic RPC framework.

---

## Edge encoding (v1)

**Format C+escape:** batched `uint16` cycle deltas with escape for large gaps.

| Raw `uint16` | Meaning |
| --- | --- |
| `1 … 0xFFFE` | delta cycles before next toggle |
| `0xFFFF` | escape: next 4 bytes LE `uint32` absolute delta |
| `0` | illegal |

Long silence must use escape (or SyncMarker), not thousands of fake edges.

Max reasonable delta: ~5 s of Apple II cycles (`1020484 * 5`).

---

## EdgeBatch wire layout

Header (10 bytes) + variable delta payload:

| Field | Size | Notes |
| --- | --- | --- |
| type | 1 | `0x10` |
| encoding | 1 | `0` = FixedU16DeltaWithEscape |
| sequence | 2 | LE, monotonic |
| baseCycle | 4 | LE, timeline at batch start |
| edgeCount | 2 | LE, 1…64 |
| deltas | 2 or 6 each | u16 or escape+u32 |

Max edges/frame (v1): **64**.

**Byte order:** all multi-byte integers are **little-endian**.

**Integer widths:** `uint8` / `uint16` / `uint32` as listed — no implicit widening on the wire.

---

## SyncMarker (9 bytes)

| Field | Size |
| --- | --- |
| type | 1 (`0x11`) |
| sequence | 2 |
| timelineCycle | 4 |
| speakerLevel | 1 (0=low, 1=high) |
| reason | 1 (0=start, 1=resync, 2=host_request) |

Establishes timeline reference + speaker state + sequence after loss/reconnect.

---

## Sequence / resync

- Monotonic `sequence` (wraps at 16-bit)
- Gap → diagnostic + soft resync (accept newer baseCycle / SyncMarker)
- Duplicate → drop batch
- Underrun → silence + drop stale edges (**no late replay**)
- Overflow → drop oldest to preserve bounded latency

Do not assume every packet arrives.

**Reorder / stale:** GATT writes on one connection are typically ordered; if a
lower sequence arrives after a higher one, treat as sequence error / drop.
**Duplicate sequence:** drop the batch; increment diagnostic.
**Unknown frame type** (including future types and `0x7F` Error unless handled):
reject as **Malformed**, drop, do not crash. Do not invent edges for gaps.

---

## Caps payload (10 bytes)

| Field | Size |
| --- | --- |
| type | 1 (`0x01`) |
| protocolVersion | 2 |
| capabilities | 4 |
| preferredEncoding | 1 |
| maxEdgesPerFrame | 2 |

---

## Control payload (3 bytes)

| Field | Size |
| --- | --- |
| type | 1 (`0x02`) |
| command | 1 |
| reserved | 1 |

---

## Callback safety

BLE write callback: decode + enqueue only.  
No SBC encode, no heavy PCM, no OLED, no NVS.

---

## Cross-project boundary

| In BlueShift | In ESP][ |
| --- | --- |
| Extension GATT server (optional) | Extension GATT client |
| Edge→PCM + A2DP | Cycle-accurate speaker edges (`SpeakerEvent` / delta encoder) |
| Classic HID Host | (unchanged) |

Shared contract: **this protocol document** + `components/protocol/*` + neutral fixtures under `test/fixtures/audio/`.

BlueShift must **not** require ESP][ to send pre-rendered PCM.  
BlueShift must **not** import the ESP][ source tree.

---

## Neutral fixtures / golden vectors

See:

- `test/fixtures/audio/README.md` — scenario fixtures
- `test/fixtures/audio/GOLDEN.md` — Protocol V1 golden wire bytes
- `test/fixtures/audio/golden_*.json` — byte-exact interoperability contract

Golden `wireHex` fields are authoritative for serialization tests (encode must
match exact bytes; do not rely only on encode→decode round-trips).
