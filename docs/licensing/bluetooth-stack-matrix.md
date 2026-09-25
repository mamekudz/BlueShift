# Bluetooth stack license & capability matrix

**Purpose:** Choose a runtime Bluetooth architecture compatible with BlueShift’s
intended community + separate commercial licensing model.  
**Not** final BlueShift license text.  
**Baseline:** post `f02a917` licensing audit.

Commercial-dependency classification (runtime deps only):

| Label | Meaning |
| --- | --- |
| **GREEN** | Suitable for intended commercial manufacturing/OEM without a second unrelated BT-stack commercial license |
| **YELLOW** | Usable but extra obligations, platform pins, or uncertainty |
| **RED** | Requires separate commercial runtime license or non-commercial-only terms |

---

## Matrix

| Criterion | ESP-IDF / Bluedroid BTDM | Bluepad32 | BTstack | ESP32KeyBridge-derived | EspBle-derived |
| --- | --- | --- | --- | --- | --- |
| Classic HID Host | **Yes** (APIs + `esp_hid_host` / `esp_bt_hid_host_*`) — ESP32 only | Yes (via BTstack) | Yes | Yes (via EspBleClassic) | Yes (`EspBleClassic`) |
| BLE HID Peripheral | **Yes** (`esp_hid_device` / `esp_hidd_dev_*` / Bluedroid GATTS HOGP) | Only if built **inside** BTstack | Yes | Yes | Yes (NimBLE HID) |
| Simultaneous Classic Host + BLE Peripheral | **Documented as design goal** on original ESP32 with `ESP_BT_MODE_BTDM` + Bluedroid Classic+BLE; official examples cover dual-mode HID host **or** HID device — **combined Host+Device roles** supported at Kconfig level after 2023 HID Host+Device co-build; BlueShift physical coexistence **UNVERIFIED** | Requires both roles on BTstack | Possible in one stack | EspBle docs: experimental dual-host with peer tests | Same family; experimental dual-host |
| ESP32-WROVER | Yes | Yes | Yes | Yes | Yes |
| License | ESP-IDF / Bluedroid: **Apache-2.0** (+ nested notices) | Bluepad32: **Apache-2.0** | Modified BSD + **non-commercial clause**; commercial from BlueKitchen | **MIT** | **MIT** (+ NimBLE Apache notices) |
| Commercial use | **GREEN** (Espressif terms) | **RED** if shipping with BTstack without BlueKitchen deal | **RED** without BlueKitchen deal | **GREEN** (MIT) | **GREEN** (MIT) with attribution |
| Attribution | Espressif NOTICE | Apache NOTICE + BTstack | BlueKitchen LICENSE | MIT notice | MIT + NimBLE |
| Implementation effort | Medium–high (profiles + dual init) | Low for Classic gamepads; high for BLE-out | High | Medium (adapt) | Medium; **Arduino ≥ 3.2** pin |
| Memory impact | High (BTDM reserve DRAM ~0xdb5c on Arduino ESP32 SDK) | High | High | High | High |
| Maturity | High (Espressif maintained) | High for Classic pads | High | Growing | Experimental dual-host |
| Risks | Arduino **prebuilt** SDK may ship **without** `CONFIG_BT_HID_*` (see below) | BTstack commercial | Forces every OEM through BlueKitchen | Still wraps Espressif stacks | Platform version; dual-host experimental |
| **Commercial classification** | **GREEN** (target production) | **RED** | **RED** | **GREEN** (MIT wrapper) | **YELLOW→GREEN** (MIT; Arduino pin) |

---

## Preference order (applied)

1. **ESP-IDF / Espressif Bluedroid BTDM** — selected production *architecture*  
2. MIT/BSD/Apache components (EspBle/KeyBridge as optional accelerators)  
3. Other commercially usable OSS  
4. **BTstack only if no reasonable alternative** — **not selected**

---

## Hard finding on current BlueShift PlatformIO pin

Platform: `espressif32 @ 6.9.0` / Arduino-ESP32 2.x prebuilt SDK for ESP32:

| Item | Evidence |
| --- | --- |
| Controller mode | `CONFIG_BTDM_CTRL_MODE_BTDM=y` |
| Classic + BLE host | `CONFIG_BT_CLASSIC_ENABLED`, `CONFIG_BT_BLE_ENABLED` |
| Classic HID profile | `# CONFIG_BT_HID_ENABLED is not set` in `sdkconfig` |
| `esp_bt_hid_host_init` in `libbt.a` | **Not present** (byte search / archive probe, 2026-09-25) |
| BLE HID device (`esp_hidd_dev_init` / `esp_ble_hidd_dev_init`) | **Present** in `libesp_hid.a` |

**Conclusion:** On the **current Arduino prebuilt package**, Classic HID Host cannot be linked. BLE HOGP peripheral symbols exist. Full dual-mode Classic Host + BLE Peripheral therefore requires either:

- switching the production env to **ESP-IDF** (or Arduino+ESP-IDF) with `sdkconfig` enabling `CONFIG_BT_HID_HOST_ENABLED` (+ BLE GATTS HID device), or  
- upgrading to a framework build that ships Classic HID Host, or  
- adopting EspBle Classic archive on a supported Arduino 3.x line (**not** silently BTstack).

This is an **Arduino packaging / sdkconfig** limitation, not proof that ESP-IDF cannot do the roles.

---

## Bluepad32 without BTstack?

| Outcome | Finding |
| --- | --- |
| **A** Runtime inseparable | Custom platforms still call `btstack_init()` / `btstack_run_loop_execute()` ([docs](https://bluepad32.readthedocs.io/en/latest/adding_new_platform/)) |
| **B** Parsers Apache-2.0 | `uni_hid_parser*.h` are Apache-2.0 but typed around `uni_hid_device_s` and Bluepad32 report pipeline — **not** a drop-in without BTstack |
| **C** Alternate backend | No Espressif-Bluedroid backend in Bluepad32 |

**BlueShift policy:** keep **BlueShift-owned** device profiles (`components/hid/device_parsers.*`). Do **not** vendor Bluepad32/BTstack. Optionally study public report layouts for evidence only.

---

## Selected architecture (this milestone)

```
ESP32 controller BTDM
        |
   Bluedroid (ESP-IDF)
   ├── Classic HID Host   → EspIdfClassicHidHost
   └── BLE HID Device     → EspIdfBleHidPeripheral
                |
         project BridgeCore / parsers
```

- **BTstack required?** **No**  
- **Commercial-dependency class:** **GREEN** (Espressif) for the target graph; **YELLOW** until Classic HID Host is enabled in the actual build product used for T-Lion  

See `docs/bluetooth/esp-idf-dual-mode.md`.
