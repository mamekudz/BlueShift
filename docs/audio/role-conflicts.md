# Triple-role / connection conflict analysis

**Status:** RESEARCH — link success ≠ runtime coexistence proven  
**Physical:** UNVERIFIED

---

## Target concurrent scenario (future)

```text
SN30 Pro  --Classic HID--> BlueShift
ESP][     <--BLE HID------ BlueShift
ESP][     --BLE Ext audio-> BlueShift
Headset   <--A2DP---------- BlueShift
```

Identities remain separate: Classic input bond ≠ BLE host bond ≠ A2DP sink bond.

---

## What LINK VERIFIED can prove

- Symbols present
- Kconfig not mutually exclusive
- One Bluedroid BTDM lifecycle compiles

What it **cannot** prove: RF airtime, ACL limits under load, audio+HID latency, memory pools at runtime.

---

## Stack facts (ESP-IDF 5.3.1 / docs + examples)

| Item | Evidence |
| --- | --- |
| A2DP Source | Official `a2dp_source` example (Classic) |
| A2DP + GATTS coex | Official `a2dp_gatts_coex` (A2DP **Sink** + GATTS on BTDM) |
| A2DP Source + HID Host + BLE HID Device | **No official example** — BlueShift spike is the experiment |
| A2DP Source connections | Docs: at most one A2DP sink |
| ACL | Raise `CONFIG_BT_ACL_CONNECTIONS` (spike uses 4) |

---

## Theoretical pressures

| Resource | Concern |
| --- | --- |
| BR/EDR ACL | HID Classic + A2DP (+ optional AVRCP) |
| BLE link | HID + Extension writes |
| Controller buffers / airtime | Classic audio is greedy |
| DRAM | Bluedroid + A2DP + HID already large |
| CPU | SBC encode vs HID forward |

---

## Isolation policy

If audio fails or starves: **HID continues**.  
Audio is OPTIONAL.

---

## Next physical tests (when T-Lion arrives)

1. Dual HID only (baseline)
2. A2DP alone to headphones
3. HID + A2DP without ESP][ audio stream
4. Full triple + edge stream
5. Latency/HID jitter under audio load
