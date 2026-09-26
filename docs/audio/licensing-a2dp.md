# A2DP Source licensing (ESP-IDF 5.3.1)

**Status:** AUDITED against local `framework-espidf @ 3.50301.0`  
**Commercial class (current graph):** **GREEN** — Espressif / Apache-2.0 (no BTstack)

---

## Components required for A2DP Source

| Piece | Location | License evidence |
| --- | --- | --- |
| Bluedroid A2DP API | `components/bt/.../esp_a2dp_api.h` | ESP-IDF Apache-2.0 |
| A2DP Source BTC | `btc_a2dp_source.*` | Apache-2.0 (Espressif/Bluedroid tree) |
| SBC encoder | `external/sbc/encoder/*` | **Apache-2.0** (Broadcom copyright headers) |
| SBC decoder | `external/sbc/decoder/*` | **Apache-2.0** (AOSP / Open Interface headers) |
| Controller lib | Espressif prebuilt | Apache-2.0 + Espressif notices |

Kconfig: `CONFIG_BT_A2DP_ENABLE` (depends on `CONFIG_BT_CLASSIC_ENABLED`).

APIs used by spikes:

- `esp_a2d_register_callback`
- `esp_a2d_source_init` / `deinit`
- `esp_a2d_source_register_data_callback`
- `esp_a2d_source_connect` / `disconnect`

---

## Not required

| Stack | Status |
| --- | --- |
| BTstack | **NOT REQUIRED** |
| NimBLE | **NOT REQUIRED** for A2DP Source |
| External proprietary SBC | **NOT REQUIRED** for internal codec path |

---

## Codec caveat

Internal A2DP path uses **SBC**. External/SEP AAC paths (if enabled later) need a separate license check — **not enabled** in BlueShift spikes.

Bluetooth **patent/royalty** questions for shipping audio products remain a legal/product decision outside “source license SPDX”. Document for counsel if commercial audio ships.

---

## Matrix update

Adding A2DP Source **does not** by itself force BTstack. Dependency graph remains **GREEN** under current Espressif Apache-2.0 components.
