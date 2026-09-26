# Native ESP-IDF dual-mode Bluetooth spike

**Status:** COMPILE VERIFIED / LINK VERIFIED for combined Classic HID Host + BLE HID Device.  
**Physical:** UNVERIFIED (T-Lion unavailable).  
**Date:** 2026-09-26

This document records the feasibility spike proving that **original ESP32** under
**native ESP-IDF** can link both required BlueShift roles in **one binary**.

It does **not** claim pairing, bridging, or board bring-up.

---

## Versions (pinned)

| Item | Value |
| --- | --- |
| PlatformIO platform | `espressif32 @ 6.9.0` |
| Framework package | `framework-espidf @ 3.50301.0` |
| ESP-IDF | **5.3.1** |
| Toolchain | `toolchain-xtensa-esp-elf @ 13.2.0+20240530` |
| Target | `esp32` (LILYGO T-Lion / WROVER board JSON) |
| Spike envs | `t-lion-idf-spike` (dual), `-classic`, `-ble` |

PlatformIO’s `espidf.py` loads **project-root** `sdkconfig.defaults` only.
`spike/idf/pio_sdkconfig_pre.py` copies the per-env defaults file before each build.

---

## Host stack decision

| Option | Classic BR/EDR | BLE HID Device | Selected |
| --- | --- | --- | --- |
| **Bluedroid BTDM** | Yes (`esp_bt_hid_host_*`) | Yes (`esp_hidd` / GATTS) | **YES** |
| NimBLE | No Classic | Yes | No — cannot satisfy Classic alone |
| Bluedroid + NimBLE | Possible mess | Dual-host conflict | **No** |
| BTstack | Yes | Yes | **Not required** (license RED) |

**One coherent stack:** Bluedroid with `ESP_BT_MODE_BTDM` / `CONFIG_BTDM_CTRL_MODE_BTDM`.

---

## Components / APIs (ESP-IDF 5.3.1 source)

### A) Classic HID Host

| | |
| --- | --- |
| Component | `bt` (Bluedroid) |
| Implementation | `components/bt/host/bluedroid/api/esp_hidh_api.c` |
| Header | `components/bt/host/bluedroid/api/include/api/esp_hidh_api.h` |
| Symbols | `esp_bt_hid_host_init`, `esp_bt_hid_host_register_callback`, `esp_bt_hid_host_connect`, `esp_bt_hid_host_disconnect` |
| Higher-level wrap | `esp_hid` → `src/bt_hidh.c` (uses Classic Host when `CONFIG_BT_HID_HOST_ENABLED`) |

### B) BLE HID Device / HOGP

| | |
| --- | --- |
| Component | `esp_hid` |
| Implementation | `components/esp_hid/src/esp_hidd.c`, `ble_hidd.c` |
| Header | `components/esp_hid/include/esp_hidd.h` |
| Symbols | `esp_hidd_dev_init`, `esp_ble_hidd_dev_init` |
| Requires | Bluedroid BLE + `CONFIG_BT_GATTS_ENABLE` |

Note: example-local `esp_hid_gap.*` is **not** a component API. The spike uses
`esp_ble_gap_*` + `esp_hidd_dev_init` only.

---

## Required Kconfig (dual)

From `spike/idf/sdkconfig.defaults.dual`:

```
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_PARTITION_TABLE_SINGLE_APP_LARGE=y
CONFIG_BT_ENABLED=y
CONFIG_BT_CONTROLLER_ENABLED=y
CONFIG_BTDM_CTRL_MODE_BTDM=y
CONFIG_BT_BLUEDROID_ENABLED=y
CONFIG_BT_CLASSIC_ENABLED=y
CONFIG_BT_BLE_ENABLED=y
CONFIG_BT_GATTS_ENABLE=y
CONFIG_BT_GATTC_ENABLE=y
CONFIG_BT_BLE_SMP_ENABLE=y
CONFIG_BT_SMP_ENABLE=y
CONFIG_BT_HID_ENABLED=y
CONFIG_BT_HID_HOST_ENABLED=y
# CONFIG_BT_NIMBLE_ENABLED is not set
```

---

## Initialization order (spike)

```
nvs_flash_init
esp_bt_controller_init / enable(ESP_BT_MODE_BTDM)
esp_bluedroid_init / enable
esp_bt_hid_host_register_callback
esp_bt_hid_host_init
esp_ble_gap_register_callback
esp_ble_gap_set_device_name
esp_hidd_dev_init(..., ESP_HID_TRANSPORT_BLE, ...)
```

Single controller enable — no competing stacks.

---

## Compile / link results

### CLASSIC_HID_HOST (`t-lion-idf-spike-classic`)

| | |
| --- | --- |
| compile | PASS (when built) |
| link | PASS (when built) |
| symbols | PRESENT (Bluedroid HID Host) |

### BLE_HID_DEVICE (`t-lion-idf-spike-ble`)

| | |
| --- | --- |
| compile | PASS (when built) |
| link | PASS (when built) |
| symbols | PRESENT (`esp_hidd_dev_init` / `esp_ble_hidd_dev_init`) |

### COMBINED (`t-lion-idf-spike`) — critical

| | |
| --- | --- |
| compile | **PASS** |
| link | **PASS** |
| Flash (app partition) | ~914172 / 1048576 (87.2%) — SINGLE_APP_LARGE |
| DRAM | ~44468 / 327680 (13.6%) |

#### ELF symbol evidence (`xtensa-esp32-elf-nm firmware.elf`)

```
T esp_bt_hid_host_init
T esp_bt_hid_host_register_callback
T esp_hidd_dev_init
T esp_ble_hidd_dev_init
```

Connect/disconnect are part of the same Bluedroid HID Host object file; the
spike also references their addresses where linked.

**Mocks were not used.**

---

## Arduino comparison (same platform pin)

| | Arduino-ESP32 2.x prebuild (`t-lion-debug`) | Native ESP-IDF spike |
| --- | --- | --- |
| Classic HID Host | **NOT LINKED** (`esp_bt_hid_host_init` absent from `libbt.a`) | **LINKED** |
| BLE HID Device | **AVAILABLE** (`libesp_hid.a`) | **LINKED** |
| Simultaneous | **NO** | **YES (compile/link)** |
| Classification | Framework/package **PARTIAL** | Architecture **GREEN** candidate |

This is **not** an original-ESP32 silicon limitation.

---

## License / commercial class

| Component | License | Class |
| --- | --- | --- |
| ESP-IDF `bt` / Bluedroid | Apache-2.0 (Espressif) | GREEN |
| ESP-IDF `esp_hid` | Apache-2.0 | GREEN |
| `nvs_flash` / controller | Apache-2.0 | GREEN |
| BTstack | — | **not used** |
| Bluepad32 | — | **not used** |

**Commercial-dependency classification for this graph: GREEN**  
BTstack required for production? **NO**

---

## Architecture consequence

Recommend migrating BlueShift **production firmware** from Arduino framework to
**native ESP-IDF** (or an ESP-IDF-based PlatformIO env) so Classic HID Host is
available with BLE HID Device under one Bluedroid BTDM configuration.

**Not done in this spike:** full application migration. Arduino envs remain the
known baseline until a reviewed migration milestone.

---

## Physical verification

Still pending T-Lion board. Allowed labels only:

- SOURCE VERIFIED / COMPILE VERIFIED / LINK VERIFIED / HOST TESTED / IMPLEMENTED_UNVERIFIED
