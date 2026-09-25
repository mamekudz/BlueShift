# ESP32KeyBridge evaluation (Milestone 2)

**Status:** researched only — **not incorporated** (code not vendored).

Source: https://github.com/tanakamasayuki/ESP32KeyBridge (MIT)

## What it is

A portable bridge core that:

- accepts keyboard-shaped input from adapters (USB Host, BLE, **Bluetooth Classic HID Host**, GPIO, …);
- normalizes to one key model;
- remaps / layers / macros;
- outputs to USB Device HID, **BLE HID**, Classic HID, serial, …

Core claims: no Arduino/ESP-IDF dependency inside the transform layer; host unit-tested; no heap in the hot path.

## Relevance to BlueShift

| BlueShift need | ESP32KeyBridge | Notes |
| --- | --- | --- |
| Classic HID Host (original ESP32) | Yes (`ESP32KeyBridgeEspBleClassic.h`, EspBle ≥ 1.3) | DOCUMENTED |
| BLE HID Peripheral | Yes (`ESP32KeyBridgeEspBle.h`) | DOCUMENTED |
| Keyboard + mouse + gamepad mapping | Present (gamepad → Key map helpers) | DOCUMENTED |
| Normalized layers | Key-centric model | Good reference; BlueShift also needs richer GamepadState |
| OLED / battery / UI | Out of scope | BlueShift-owned |
| License | MIT | Friendly for evaluation / selective reuse |

## Adapter stack

Depends on sibling libs:

- [EspBle](https://github.com/tanakamasayuki/EspBle) — NimBLE-based BLE; Classic via `EspBleClassic` on original ESP32
- EspUsbHost / EspUsbDevice — USB roles (not required for BlueShift Classic→BLE)

EspBle documents **dual-host BLE+Classic as experimental** but reports hardware passes for LE connection alongside Classic HID traffic (constraints remain).

## Code reuse policy

BlueShift may:

- adopt **architectural patterns** (raw → normalize → output; release-on-disconnect; bounded state);
- later evaluate **dependency** on EspBle / selective adapters;

BlueShift must **not** silently fork large unmarked copies. Any incorporation goes through `THIRD_PARTY_LICENSES.md`.

## Recommendation

Treat ESP32KeyBridge + EspBle as the **primary researched architecture candidate** for Milestone 2/3 stack selection, with BlueShift-owned `ClassicHidHost` / `BleHidPeripheral` / `BridgeCore` interfaces wrapping whatever concrete stack is chosen after physical experiments.
