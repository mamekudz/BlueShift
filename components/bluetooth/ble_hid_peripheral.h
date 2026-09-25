#pragma once

#include "bluetooth/hid_transport.h"
#include "hid/ble_hid_reports.h"

namespace blueshift {

// BLE HID peripheral spike (keyboard + gamepad reports).
// Backend: NimBLE on ARDUINO+T-Lion when BLUESHIFT_ENABLE_NIMBLE=1.
// Otherwise mock/null that still exercises report builders.
// IMPLEMENTED_UNVERIFIED.

class BleHidPeripheralSpike : public BleHidPeripheral {
public:
    void setDeviceNames(const char *keyboardName, const char *gamepadName);

    bool start() override;
    void stop() override;
    bool startPairing(uint32_t timeoutMs) override;
    bool forgetHost() override;
    BlePeripheralState getState() const override {
        return state_;
    }

    bool sendKeyboard(const KeyboardState &state) override;
    bool sendMouse(const MouseState &state) override;
    bool sendGamepad(const GamepadState &state) override;

    void tick(uint32_t nowMs);
    void mockConnect();
    void mockDisconnect();

    // Last serialized reports (for host tests / diagnostics).
    const uint8_t *lastKeyboardReport() const {
        return lastKb_;
    }
    const uint8_t *lastGamepadReport() const {
        return lastGp_;
    }

    uint32_t keyboardSendCount() const {
        return kbCount_;
    }
    uint32_t gamepadSendCount() const {
        return gpCount_;
    }
    uint32_t droppedWhileDisconnected() const {
        return dropped_;
    }

private:
    BlePeripheralState state_ = BlePeripheralState::Idle;
    char kbName_[24] = "BlueShift Keyboard";
    char gpName_[24] = "BlueShift Gamepad";
    uint32_t pairTimeoutMs_ = 60000;
    uint32_t pairDeadlineMs_ = 0;
    bool pairing_ = false;
    uint8_t lastKb_[8] = {};
    uint8_t lastGp_[9] = {};
    uint32_t kbCount_ = 0;
    uint32_t gpCount_ = 0;
    uint32_t dropped_ = 0;
};

using MockBleHidPeripheral = BleHidPeripheralSpike;

} // namespace blueshift
