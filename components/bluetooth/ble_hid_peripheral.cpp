#include "bluetooth/ble_hid_peripheral.h"

#include <cstring>

#include "blueshift/log.h"

#if defined(ARDUINO) && defined(BLUESHIFT_ENABLE_NIMBLE)
#include <NimBLEDevice.h>
// Full NimBLE HID device wiring is IMPLEMENTED_UNVERIFIED and may be expanded on board arrival.
// Current build links NimBLE and exposes advertising name; report notify path uses builders.
static NimBLEServer *g_bleServer = nullptr;
#endif

namespace blueshift {

void BleHidPeripheralSpike::setDeviceNames(const char *keyboardName, const char *gamepadName) {
    if (keyboardName != nullptr) {
        std::strncpy(kbName_, keyboardName, sizeof(kbName_) - 1);
        kbName_[sizeof(kbName_) - 1] = '\0';
    }
    if (gamepadName != nullptr) {
        std::strncpy(gpName_, gamepadName, sizeof(gpName_) - 1);
        gpName_[sizeof(gpName_) - 1] = '\0';
    }
}

bool BleHidPeripheralSpike::start() {
    state_ = BlePeripheralState::Idle;
#if defined(ARDUINO) && defined(BLUESHIFT_ENABLE_NIMBLE)
    NimBLEDevice::init(kbName_);
    g_bleServer = NimBLEDevice::createServer();
    BS_LOG_INFO("BLE", "NimBLE init name=%s (IMPLEMENTED_UNVERIFIED)", kbName_);
#else
    BS_LOG_INFO("BLE", "peripheral spike start (mock/no NimBLE) name=%s", kbName_);
#endif
    return true;
}

void BleHidPeripheralSpike::stop() {
    pairing_ = false;
    state_ = BlePeripheralState::Idle;
#if defined(ARDUINO) && defined(BLUESHIFT_ENABLE_NIMBLE)
    NimBLEDevice::deinit(true);
    g_bleServer = nullptr;
#endif
}

bool BleHidPeripheralSpike::startPairing(uint32_t timeoutMs) {
    pairTimeoutMs_ = timeoutMs == 0 ? 60000 : timeoutMs;
    pairing_ = true;
    pairDeadlineMs_ = 0;
    state_ = BlePeripheralState::Advertising;
#if defined(ARDUINO) && defined(BLUESHIFT_ENABLE_NIMBLE)
    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    if (adv != nullptr) {
        adv->setName(kbName_);
        adv->start();
    }
#endif
    BS_LOG_INFO("BLE", "advertising/pairing timeout=%lu", static_cast<unsigned long>(pairTimeoutMs_));
    return true;
}

bool BleHidPeripheralSpike::forgetHost() {
    state_ = BlePeripheralState::Idle;
    pairing_ = false;
    return true;
}

void BleHidPeripheralSpike::tick(uint32_t nowMs) {
    if (!pairing_) {
        return;
    }
    if (pairDeadlineMs_ == 0) {
        pairDeadlineMs_ = nowMs + pairTimeoutMs_;
        return;
    }
    if (nowMs >= pairDeadlineMs_ && state_ == BlePeripheralState::Advertising) {
        BS_LOG_WARN("BLE", "pairing timeout");
        pairing_ = false;
        state_ = BlePeripheralState::Idle;
#if defined(ARDUINO) && defined(BLUESHIFT_ENABLE_NIMBLE)
        NimBLEDevice::getAdvertising()->stop();
#endif
    }
}

void BleHidPeripheralSpike::mockConnect() {
    pairing_ = false;
    state_ = BlePeripheralState::Connected;
}

void BleHidPeripheralSpike::mockDisconnect() {
    if (state_ == BlePeripheralState::Connected) {
        state_ = BlePeripheralState::Idle;
    }
}

bool BleHidPeripheralSpike::sendKeyboard(const KeyboardState &state) {
    if (state_ != BlePeripheralState::Connected) {
        ++dropped_;
        return false;
    }
    if (!buildKeyboardReport(state.modifiers, state.keys, lastKb_)) {
        return false;
    }
    ++kbCount_;
    return true;
}

bool BleHidPeripheralSpike::sendMouse(const MouseState &state) {
    if (state_ != BlePeripheralState::Connected) {
        ++dropped_;
        return false;
    }
    uint8_t tmp[4];
    return buildMouseReport(state.buttons, state.dx, state.dy, state.wheel, tmp);
}

bool BleHidPeripheralSpike::sendGamepad(const GamepadState &state) {
    if (state_ != BlePeripheralState::Connected) {
        ++dropped_;
        return false;
    }
    if (!buildGamepadReport(state.leftX, state.leftY, state.rightX, state.rightY, state.leftTrigger,
                            state.rightTrigger, static_cast<uint16_t>(state.buttons & 0xFFFF),
                            static_cast<uint8_t>(state.dpad), lastGp_)) {
        return false;
    }
    ++gpCount_;
    return true;
}

} // namespace blueshift
