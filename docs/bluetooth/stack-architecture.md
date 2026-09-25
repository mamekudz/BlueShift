# Bluetooth stack architecture (Milestone 2)

**Goal:** Classic BR/EDR HID Host + BLE HID Peripheral on one original ESP32 (T-Lion).

Nothing below is physically VERIFIED on BlueShift hardware.

## Requirement

```
Classic HID device  --(BR/EDR Host)-->  BlueShift  --(BLE HID Peripheral)-->  BLE host (e.g. ESP][)
```

Original ESP32 is mandatory for Classic (ESP32-S3 has no BR/EDR).

## Candidate A — EspBle dual-host (recommended research lead)

**Stack:** EspBle (bundled NimBLE host on original ESP32) + EspBleClassic (namespaced Bluedroid Classic).

| Pros | Cons |
| --- | --- |
| Same vendor family as ESP32KeyBridge | Dual-host marked **experimental** |
| HID Device + HID Host APIs for BLE and Classic | Arduino-ESP32 version constraints (Classic ≥ 3.2 / audio higher) |
| Documented HID report packing shared across transports | RAM/HCI buffer sharing limitations |
| MIT-oriented ecosystem | Not yet in BlueShift `lib_deps` |

**Feasibility:** DOCUMENTED as intentional dual-host with hardware peer tests for LE + Classic HID coexistence (EspBle docs). Still ASSUMED for BlueShift’s specific gamepad→BLE gamepad load.

**Licensing:** EspBle MIT + bundled NimBLE / Bluedroid notices — must be recorded before adopt.

## Candidate B — Bluepad32 / BTstack + BTstack BLE HID

| Pros | Cons |
| --- | --- |
| Excellent Classic gamepad coverage | BLE HID peripheral must be built inside BTstack |
| Mature controller database | Conflicts with Arduino BLE / NimBLE wrappers |
| | Larger integration / licensing diligence |

**Feasibility:** Classic Host DOCUMENTED; simultaneous BLE Peripheral ASSUMED only if implemented on BTstack.

## Candidate C — Stock Bluedroid Classic HID Host + NimBLE Peripheral

Arduino-ESP32 on original ESP32 historically ships Bluedroid, not NimBLE. Running both Classic Bluedroid Host and a second BLE host is the hard problem EspBle solves by bundling NimBLE + Classic archive.

**Feasibility:** UNKNOWN / high risk without EspBle-like dual-host work.

## Alternatives rejected for now

| Idea | Why not |
| --- | --- |
| Two ESP32s (Classic on one, BLE on other) | Defeats single T-Lion product goal |
| ESP32-S3 only | No Classic BR/EDR |
| Wi-Fi tunnel instead of BLE HID | Breaks “appears as HID device” goal |

## RAM / resource estimates (order-of-magnitude)

| Component | Estimate | Confidence |
| --- | --- | --- |
| Dual Bluetooth hosts | High — multi-hundred KB flash, significant IRAM/DRAM | ASSUMED |
| OLED SSD1306 + UI | Low–moderate | ASSUMED |
| Bridge + queues | Low if bounded and no heap in HID path | ASSUMED |
| Headroom on WROVER + PSRAM | Likely adequate if PSRAM present | ASSUMED |

Physical measurement required before release claims.

## BlueShift software layering (decided for prep)

```
ClassicHidHost (interface)     BleHidPeripheral (interface)
        \                         /
         \                       /
          --->  BridgeCore  <---
                     |
              Normalized HID
```

Concrete stacks stay behind adapters. BridgeCore / HID models / queues are host-testable without radio.

## Milestone 3 interim decision

See [`m3-stack-decision.md`](m3-stack-decision.md): Classic/BLE **spikes** compile; dual-radio stack **not** default-linked on espressif32 6.9.0 / Arduino 2.x.

## Recommended next physical experiments (when board arrives)

1. Bring-up OLED + 5-way + battery ADC (no BT).
2. Classic HID Host only (one gamepad) with chosen stack.
3. BLE HID Peripheral only (keyboard or gamepad) to Windows/ESP][.
4. Dual-host coexistence stress (reconnect loops, both links up).
5. Full bridge with release-on-disconnect + bounded queues.

## Decision status

**No forced production stack pin yet.**

Research lead: **Candidate A (EspBle dual-host / ESP32KeyBridge patterns)**.

Fallback research: **Candidate B (Bluepad32 + BTstack BLE HID)**.

Milestone 2 implements interfaces, normalized HID, bridge FSM, and docs only — no Bluepad32/EspBle `lib_deps` until a dedicated stack spike.
