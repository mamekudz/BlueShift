#pragma once

// Build-time Bluetooth capability probes for BlueShift.
// Production ESP-IDF builds should set CONFIG_BT_HID_HOST_ENABLED in sdkconfig.

#if defined(CONFIG_BT_HID_HOST_ENABLED) || defined(CONFIG_BT_HID_ENABLED)
#define BLUESHIFT_HAVE_CLASSIC_HID_HOST_API 1
#else
#define BLUESHIFT_HAVE_CLASSIC_HID_HOST_API 0
#endif

#if defined(ARDUINO) && defined(ESP32)
#define BLUESHIFT_HAVE_BLE_HID_DEVICE_API 1
#else
#define BLUESHIFT_HAVE_BLE_HID_DEVICE_API 0
#endif
