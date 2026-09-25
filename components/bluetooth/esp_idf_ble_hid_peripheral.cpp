#include "bluetooth/esp_idf_ble_hid_peripheral.h"

#include "blueshift/log.h"
#include "hid/ble_hid_reports.h"

#if defined(ARDUINO) && defined(ESP32) && defined(BLUESHIFT_ENABLE_ESP_IDF_BLE_HID)
#include <esp_hidd.h>
#include <esp_hid_common.h>
#endif

namespace blueshift {

bool EspIdfBleHidPeripheral::start() {
    (void)BluetoothPlatform::instance().initializeDualMode();
    if (!BleHidPeripheralSpike::start()) {
        return false;
    }

#if defined(ARDUINO) && defined(ESP32) && defined(BLUESHIFT_ENABLE_ESP_IDF_BLE_HID)
    // Full HOGP init is large; enable explicitly via build flag after descriptors are finalized.
    // Default builds keep report-builder path to control flash until bring-up.
    BS_LOG_INFO("BLE", "EspIdfBleHidPeripheral: BLUESHIFT_ENABLE_ESP_IDF_BLE_HID set — "
                       "HOGP wiring pending descriptor registration (UNVERIFIED)");
    hiddReady_ = false;
#else
    BS_LOG_INFO("BLE", "EspIdfBleHidPeripheral: using report-builder path "
                       "(enable BLUESHIFT_ENABLE_ESP_IDF_BLE_HID for HOGP)");
    hiddReady_ = false;
#endif
    return true;
}

bool EspIdfBleHidPeripheral::startPairing(uint32_t timeoutMs) {
    return BleHidPeripheralSpike::startPairing(timeoutMs);
}

bool EspIdfBleHidPeripheral::sendKeyboard(const KeyboardState &state) {
    // Always keep serialization for tests / diagnostics.
    if (!BleHidPeripheralSpike::sendKeyboard(state)) {
        return false;
    }
#if defined(ARDUINO) && defined(ESP32) && defined(BLUESHIFT_ENABLE_ESP_IDF_BLE_HID)
    if (hiddReady_) {
        // esp_hidd_dev_input_set(...) when device handle is live.
    }
#else
    (void)hiddReady_;
#endif
    return true;
}

bool EspIdfBleHidPeripheral::sendGamepad(const GamepadState &state) {
    if (!BleHidPeripheralSpike::sendGamepad(state)) {
        return false;
    }
    return true;
}

} // namespace blueshift
