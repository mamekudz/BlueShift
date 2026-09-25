# Bluepad32 evaluation (Milestone 2)

**Status:** researched only — **not incorporated** into BlueShift firmware.

Sources:

- https://github.com/ricardoquesada/bluepad32
- https://bluepad32.readthedocs.io/
- https://bluepad32.readthedocs.io/en/stable/FAQ/
- Community reports on stack conflicts with Bluedroid BLE libraries

## Fit for BlueShift

| Question | Finding | Confidence |
| --- | --- | --- |
| Original ESP32? | Yes — Classic BR/EDR controllers require original ESP32 | DOCUMENTED |
| Classic gamepad host? | Strong — DualSense, Switch, 8BitDo, etc. | DOCUMENTED |
| SN30 Pro / Nimbus? | Likely via Classic support; physical confirmation later | ASSUMED |
| BLE HID peripheral coexistence? | Bluepad32 uses **BTstack**, not Bluedroid/Arduino BLE wrappers | DOCUMENTED |
| Drop-in with BleMouse / Arduino BLE? | Generally **incompatible** without implementing BLE inside BTstack | DOCUMENTED |

## Architecture implications

Bluepad32 replaces the default Bluetooth stack with BTstack. Combining it with a separate NimBLE/Bluedroid BLE HID peripheral library on one ESP32 is a known conflict pattern.

Viable use of Bluepad32 for BlueShift would require:

1. Classic HID Host via Bluepad32/BTstack, **and**
2. BLE HID Peripheral implemented **on the same BTstack**, not via Arduino BLE / NimBLE Arduino wrappers.

That is substantial custom BTstack work (GATT HID service, descriptors, bonding).

## Licensing

Bluepad32 itself is **Apache-2.0** (upstream `LICENSE`, © Ricardo Quesada).

It **depends on BTstack** (BlueKitchen). Upstream notice: BTstack has a
commercial license and is described as free for open-source projects; for
ESP32 commercial products Bluepad32 docs direct users to contact BlueKitchen.

**BlueShift status:** researched only — **not** a dependency, **not** vendored,
**not** linked. See `docs/licensing/license-audit.md`.

Before any incorporation:

- pin exact Bluepad32 + BTstack versions;
- record notices in `THIRD_PARTY_LICENSES.md`;
- obtain BlueKitchen commercial terms if shipping commercial/closed products.

## Recommendation

**Do not add Bluepad32 to production BlueShift yet.**

Keep as:

- strong reference for Classic gamepad parsing / device quirks;
- alternate architecture if EspBle dual-host proves inadequate for specific controllers.

Preferred primary path for simultaneous Classic HID Host + BLE HID Peripheral: **EspBle / ESP32KeyBridge family** (see sibling docs).
