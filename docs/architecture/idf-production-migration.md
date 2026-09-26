# Native ESP-IDF production migration notes

**Status:** PRODUCTION CANDIDATE LINK VERIFIED / IMPLEMENTED_UNVERIFIED radio & peripherals  
**Physical:** T-Lion ordered, not arrived — nothing PHYSICALLY VERIFIED.

## Decision

Native ESP-IDF (5.3.1) is the **intended production** architecture after dual-mode LINK VERIFIED.

Arduino envs (`skeleton`, `t-lion-debug`, `t-lion-release`) remain **LEGACY / BASELINE / COMPARISON**.

## Environments

| Env | Role |
| --- | --- |
| `t-lion-idf-debug` | Production candidate (debug) — LINK VERIFIED |
| `t-lion-idf-release` | Production candidate (release) — LINK VERIFIED |
| `t-lion-idf-spike*` | Minimal Bluetooth proofs only (not production sources) |

Pinned: `espressif32 @ 6.9.0`, `framework-espidf @ 3.50301.0` (= ESP-IDF 5.3.1).

## sdkconfig

Assembled by `spike/idf/pio_sdkconfig_pre.py` from:

- `sdkconfig.d/defaults.common`
- `sdkconfig.d/defaults.t-lion`
- `sdkconfig.d/defaults.debug` or `defaults.release`

Bluetooth: BTDM + Bluedroid + Classic HID Host + BLE GATTS HID Device. No NimBLE. No BTstack. No Wi-Fi init in normal production.

## Application entry

`src/idf_app/app_main.cpp` → `Application::begin()` + one FreeRTOS loop task (`bs_app`).

## Native backends (IMPLEMENTED_UNVERIFIED)

| Subsystem | Implementation |
| --- | --- |
| OLED | `IdfSsd1306Display` (project-owned I2C SSD1306) |
| Nav | `GpioNavigationBackend` ESP-IDF GPIO |
| Battery | `AdcBatteryBackend` oneshot ADC + optional eFuse cali |
| Config | NVS namespace `blueshift` |
| Device meta | NVS namespace `bs_dev` (not bond material) |
| Factory reset | scoped clear — not `nvs_flash_erase()` |
| BT lifecycle | `BluetoothPlatform` + `BluetoothError` |
| Reconnect | `ReconnectPolicy` bounded backoff |

## Evidence preserved

| Commit / artifact | Meaning |
| --- | --- |
| `3dd1ca3` | branding |
| `408f607` | native ESP-IDF dual-mode spike LINK VERIFIED |

## Related docs

- `docs/bluetooth/esp-idf-dual-mode-spike.md`
- `docs/architecture/flash-budget.md`
- `docs/architecture/memory-budget.md`
- `docs/bluetooth/reconnect-policy.md`
- `docs/testing/t-lion-bringup.md`
