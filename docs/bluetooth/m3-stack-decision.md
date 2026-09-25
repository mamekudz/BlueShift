# Milestone 3 stack decision

**Date context:** pre-physical T-Lion.

## Chosen interim path

| Layer | Implementation | Radio |
| --- | --- | --- |
| Classic HID Host | `ClassicHidHostSpike` state machine + raw inject | **No Classic radio linked** (stub) |
| BLE HID Peripheral | `BleHidPeripheralSpike` + report builders; optional NimBLE | NimBLE **off by default** |
| Bridge / parsers / UI | Real project code | Host-testable |

## Why not dual-host yet

EspBle dual-host (Milestone 2 lead) requires Arduino-ESP32 **≥ 3.2** class support for Classic companion archives. Current pin:

- PlatformIO `espressif32 @ 6.9.0`
- Arduino 2.x framework line

Enabling NimBLE-Arduino **and** Bluedroid Classic Host together on this pin is high-risk stack duplication (Milestone 3 §41).

## Blocker (exact)

> Dual Classic Host + BLE Peripheral on one ESP32 needs EspBle-style coexistence **or** a platform upgrade. Not proven on BlueShift’s current PlatformIO pin.

## What still compiles / tests

- End-to-end synthetic bridge (parser → BridgeCore → BLE report bytes)
- Stuck-key / disconnect safety
- Pairing timeouts in spikes
- OLED / nav / ADC backends for documented pins

## Next when board + platform allow

1. Upgrade path evaluation to EspBle dual-host **or**
2. BTstack-only Classic+BLE peripheral (license audit first)
3. Re-measure flash/DRAM (`docs/architecture/memory-budget.md`)
