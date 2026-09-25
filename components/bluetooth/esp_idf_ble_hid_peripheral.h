#pragma once

#include "bluetooth/ble_hid_peripheral.h"
#include "bluetooth/bluetooth_platform.h"

namespace blueshift {

// ESP-IDF / Bluedroid BLE HID Device (HOGP) adapter.
// Uses esp_hidd when available; otherwise falls back to report-builder spike.
// IMPLEMENTED_UNVERIFIED.

class EspIdfBleHidPeripheral final : public BleHidPeripheralSpike {
public:
    bool start() override;
    bool startPairing(uint32_t timeoutMs) override;
    bool sendKeyboard(const KeyboardState &state) override;
    bool sendGamepad(const GamepadState &state) override;

    bool hardwareApiPresent() const {
        return BluetoothPlatform::instance().bleHidDeviceAvailable();
    }

private:
    bool hiddReady_ = false;
};

} // namespace blueshift
