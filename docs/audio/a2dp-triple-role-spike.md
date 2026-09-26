# A2DP + triple-role spike results

**Status:** COMPILE / LINK VERIFIED for spikes — PHYSICAL UNVERIFIED  
**Pinned:** ESP-IDF 5.3.1 / `framework-espidf @ 3.50301.0` / `espressif32 @ 6.9.0`

---

## Init order (spike)

```text
NVS
controller BTDM (or Classic-only for A2DP-only spike)
Bluedroid init/enable
Classic HID Host init
BLE GAP + esp_hidd_dev_init
A2DP Source register + esp_a2d_source_init
```

Single controller / Bluedroid lifecycle — adapters must not re-init.

---

## Environments

| Env | Mode | Result |
| --- | --- | --- |
| `t-lion-idf-spike` | dual HID | **SUCCESS** (regression) |
| `t-lion-idf-spike-a2dp` | A2DP Source | **SUCCESS** |
| `t-lion-idf-spike-triple` | HID Host + BLE HID + A2DP | **SUCCESS** |

---

## ELF evidence (`t-lion-idf-spike-triple`)

```text
CLASSIC_HID_HOST    LINKED  esp_bt_hid_host_{init,connect,disconnect} (T)
BLE_HID_DEVICE      LINKED  esp_hidd_dev_init / esp_ble_hidd_dev_init (T)
A2DP_SOURCE         LINKED  esp_a2d_source_{init,register_data_callback,connect,disconnect} (T)
```

`nm` on `.pio/build/t-lion-idf-spike-triple/firmware.elf` (BUILD MEASURED 2026-09-26).

---

## Size table (BUILD MEASURED — temporary ~1 MiB `SINGLE_APP_LARGE` slot)

| Env | Flash | DRAM | Notes |
| --- | --- | --- | --- |
| dual HID | 914852 / 1048576 (**87.2%**) | 44476 (13.6%) | baseline preserved |
| A2DP-only | 720804 / 1048576 (**68.7%**) | 50848 (15.5%) | Classic BR/EDR |
| **triple** | **1038524 / 1048576 (99.0%)** | **60724 (18.5%)** | fits spike slot barely |

**Partition impact:** Triple-role must **not** rely on the temporary 1 MiB spike slot for production. Use `partitions/t-lion-no-ota.csv` (~1.9 MiB) or larger — see `docs/architecture/flash-budget.md`.

---

## Licensing

See [`licensing-a2dp.md`](licensing-a2dp.md) — Apache-2.0 SBC encoder/decoder in-tree; BTstack not required; class **GREEN**.
