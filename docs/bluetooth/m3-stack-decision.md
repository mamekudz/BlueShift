# Milestone 3 interim → Milestone 4 stack decision

**Superseded for production architecture by:** [`esp-idf-dual-mode.md`](esp-idf-dual-mode.md)

## Interim (M3)

| Layer | Implementation | Radio |
| --- | --- | --- |
| Classic / BLE | Spikes + mocks | No dual radio linked |

## Production candidate (M4)

| Layer | Implementation | License class |
| --- | --- | --- |
| Platform | `BluetoothPlatform` Bluedroid BTDM | GREEN (Espressif) |
| Classic | `EspIdfClassicHidHost` | GREEN |
| BLE out | `EspIdfBleHidPeripheral` | GREEN |
| BTstack / Bluepad32 | **Not used** | Would be RED |

Arduino 2.x package still lacks Classic HID Host symbols — Partial until ESP-IDF sdkconfig.
