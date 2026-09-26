# Flash / partition budget — BlueShift native ESP-IDF

**Status:** BUILD MEASURED where noted; **PHYSICAL VERIFIED: none** (T-Lion pending).

Pinned stack: ESP-IDF **5.3.1** / `framework-espidf @ 3.50301.0` / `espressif32 @ 6.9.0`.

---

## Documented vs assumed flash

| Item | Value | Label |
| --- | --- | --- |
| Module | ESP32-WROVER | DOCUMENTED |
| Flash size | 4 MB SPI | ASSUMED (marketing + typical WROVER); UNKNOWN exact SKU until chip probe |
| PSRAM | 8 MB | ASSUMED marketing; UNKNOWN until `esp_psram_get_size()` on board |

Runtime chip info (`captureChipInfo` / `emitHwRevisionReport`) compares DETECTED vs DOCUMENTED on first bring-up. Do not auto-rewrite source config from runtime findings.

---

## Why ~87% was not a capacity crisis

Dual-mode spike measured ~**915 KiB** application against a temporary **~1 MiB** `SINGLE_APP_LARGE` slot (~87%).

That slot was a **partition choice**, not the physical 4 MB flash limit.

---

## Partition strategy

| Layout | File | App slot | Default? | Notes |
| --- | --- | --- | --- | --- |
| No OTA / large factory | `partitions/t-lion-no-ota.csv` | **~1920 KiB** (`0x1E0000`) | **Release** | Recommended production |
| Debug large | `partitions/t-lion-debug.csv` | **~2176 KiB** | Debug | Extra coredump room |
| Dual OTA | `partitions/t-lion-ota.csv` | 2 × ~1792 KiB | No | Optional future |

NVS + PHY + optional coredump / nvs_keys reserved. No filesystem partition without a real requirement.

---

## OTA decision

**Recommendation: USB firmware update only for V1.**

Reasons:

- BlueShift is a dedicated Classic→BLE adapter; Wi-Fi is not core.
- Dual OTA halves usable app flash and pressures the Bluedroid dual-mode image.
- Optional OTA may be revisited later as an explicit feature with optional Wi-Fi.

---

## Wi-Fi policy

Do **not** initialize Wi-Fi in normal production firmware.

Avoid RAM, power, attack surface, and complexity. Keep Wi-Fi optional if OTA is ever adopted.

---

## Build-measured sizes

| Env | App flash | DRAM | Notes | Label |
| --- | --- | --- | --- | --- |
| t-lion-idf-spike (dual) | 914852 / 1048576 (87.2%) | 44476 / 327680 (13.6%) | ~1 MiB slot | BUILD MEASURED 2026-09-26 |
| t-lion-idf-spike-a2dp | 720804 / 1048576 (68.7%) | 50848 / 327680 (15.5%) | A2DP Source only | BUILD MEASURED 2026-09-26 |
| **t-lion-idf-spike-triple** | **1038524 / 1048576 (99.0%)** | **60724 / 327680 (18.5%)** | HID+HID+A2DP | BUILD MEASURED 2026-09-26 |
| **t-lion-idf-debug** | **1026875 / 2228224 (46.1%)** | **48048 / 327680 (14.7%)** | debug partition ~2.1 MiB | BUILD MEASURED 2026-09-26 |
| **t-lion-idf-release** | **852596 / 1966080 (43.4%)** | **46392 / 327680 (14.2%)** | no-OTA ~1.9 MiB | BUILD MEASURED 2026-09-26 |
| t-lion-debug (Arduino) | ~1137 KiB / ~1.25 MiB | ~41 KiB | LEGACY baseline | BUILD MEASURED |

Production headroom with no-OTA layout: **>1 MiB** free in app slot after dual-mode production link — BUILD MEASURED (not PHYSICAL VERIFIED).

---

## Growth margin (target)

| Area | Notes |
| --- | --- |
| Bluetooth stack | Dominant flash consumer |
| OLED / UI / i18x | Modest |
| Profiles / diagnostics / config | Modest |
| Future margin | Prefer no-OTA until size forces OTA redesign |
