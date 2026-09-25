# ESP-IDF dual-mode Bluetooth candidate (Classic HID Host + BLE HID Peripheral)

**Status:** architecture **SELECTED**; physical dual-role coexistence **IMPLEMENTED_UNVERIFIED** / not yet radio-proven on T-Lion.  
**Commercial-dependency class:** **GREEN** (Espressif Apache-2.0) for the intended dependency graph.  
**BTstack:** **not required** — remains EVALUATED / REFERENCE ONLY.

---

## Target roles

| Role | Transport | ESP-IDF evidence |
| --- | --- | --- |
| Input | Classic BR/EDR **HID Host** | `esp_bt_hid_host_*` / `esp_hid` host path; example `examples/bluetooth/esp_hid_host` |
| Output | BLE **HID Device** (HOGP) | `esp_hidd_dev_*`; examples `esp_hid_device`, `bluedroid/ble/ble_hid_device_demo` |
| Controller | Dual-mode | `ESP_BT_MODE_BTDM` on original ESP32 |

Chip requirement: **original ESP32** (WROVER). ESP32-S3 has no Classic BR/EDR.

---

## Simultaneous operation — what is vs is not evidenced

### Documented

- Original ESP32 controller supports BTDM (Classic + BLE).  
- Bluedroid supports Classic + BLE enabled together.  
- Espressif provides HID Host and HID Device APIs and examples.  
- Kconfig evolution allows Classic HID Host and Classic HID Device in one binary (2023+).  
- BLE HID Device uses GATTS; Classic HID Host uses HID profile — different layers.

### Not claimed VERIFIED for BlueShift

- Stable **Classic HID Host + BLE HID Peripheral** under gamepad→keyboard/gamepad bridge load.  
- Bonding DB coexistence under reconnect stress.  
- Exact ACL/BLE connection budget for BlueShift product use.

Label physical results when the board arrives.

---

## Required sdkconfig (production ESP-IDF / custom SDK)

Documented intent — do **not** rely on silent defaults:

```
CONFIG_BT_ENABLED=y
CONFIG_BTDM_CTRL_MODE_BTDM=y
CONFIG_BT_BLUEDROID_ENABLED=y
CONFIG_BT_CLASSIC_ENABLED=y
CONFIG_BT_BLE_ENABLED=y
CONFIG_BT_GATTS_ENABLE=y
CONFIG_BT_SMP_ENABLE=y
CONFIG_BT_HID_ENABLED=y
CONFIG_BT_HID_HOST_ENABLED=y
# BLE HID device via esp_hid / GATTS (Bluedroid)
# Keep NimBLE disabled when using Bluedroid HOGP to avoid dual-host conflict
# CONFIG_BT_NIMBLE_ENABLED is not set
```

Bonding / NVS: use Bluedroid defaults + BlueShift `ConfigStore` for app settings; Classic/BLE bonds remain in BT NVS — factory reset must call stack forget hooks (already sketched).

---

## Arduino-ESP32 2.x / PlatformIO 6.9.0 packaging blocker

On the **prebuilt** ESP32 Arduino SDK used by BlueShift today:

- BTDM + Classic + BLE are on.  
- **`CONFIG_BT_HID_ENABLED` is not set.**  
- Symbol `esp_bt_hid_host_init` is **absent** from `libbt.a` (local probe 2026-09-25).  
- BLE HID device symbols (`esp_hidd_dev_init`, `esp_ble_hidd_dev_init`) **are** present in `libesp_hid.a`.

Therefore `t-lion-debug` (Arduino) can compile BlueShift **adapters**, but Classic Host radio path must remain a **compile-time stub** until the firmware is built against an SDK with Classic HID Host enabled (ESP-IDF env or rebuilt Arduino libs).

**Do not** “fix” this by pulling BTstack.

---

## BlueShift software mapping

```
BluetoothPlatform::initializeDualMode()
        ├── EspIdfClassicHidHost  (ClassicHidHost)
        └── EspIdfBleHidPeripheral (BleHidPeripheral)
                    │
              BridgeCore + device_parsers
```

No ESP-IDF types leak into `BridgeCore`.

---

## EspBle / ESP32KeyBridge

- **License:** MIT — commercially friendly wrappers.  
- **Evidence:** intentional dual-host experiments; useful as reference.  
- **Code in BlueShift:** still none copied.  
- **Pin:** Classic companion often needs newer Arduino-ESP32 than 2.x.

Prefer pure Espressif APIs first; consider EspBle only if ESP-IDF bring-up stalls **and** licensing stays GREEN.

---

## Next physical tests (when T-Lion arrives)

1. ESP-IDF (or HID-enabled SDK) build with options above.  
2. Classic HID Host ↔ SN30 / keyboard only.  
3. BLE HID Peripheral ↔ Windows / ESP][ only.  
4. Both links up + reconnect stress.  
5. Memory/runtime measurement (beyond linker sizes).
