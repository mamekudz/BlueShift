# ESP-IDF Bluetooth sdkconfig defaults (production intent)

Copy these into an ESP-IDF `sdkconfig.defaults` (or equivalent) when leaving the
Arduino prebuilt SDK that omits Classic HID Host.

Do **not** enable NimBLE alongside Bluedroid HOGP for BlueShift.

```
CONFIG_BT_ENABLED=y
CONFIG_BTDM_CTRL_MODE_BTDM=y
CONFIG_BT_BLUEDROID_ENABLED=y
CONFIG_BT_CLASSIC_ENABLED=y
CONFIG_BT_BLE_ENABLED=y
CONFIG_BT_GATTS_ENABLE=y
CONFIG_BT_BLE_SMP_ENABLE=y
CONFIG_BT_SMP_ENABLE=y
CONFIG_BT_HID_ENABLED=y
CONFIG_BT_HID_HOST_ENABLED=y
# CONFIG_BT_NIMBLE_ENABLED is not set
```

See `docs/bluetooth/esp-idf-dual-mode.md` and `docs/licensing/bluetooth-stack-matrix.md`.
