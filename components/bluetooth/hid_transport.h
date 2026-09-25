#pragma once

#include <cstdint>

#include "hid/normalized_hid.h"

namespace blueshift {

enum class ClassicHostState : uint8_t {
    Idle = 0,
    Pairing,
    Connected,
    Reconnecting,
    Error
};

// Project-owned Classic HID Host interface — no Bluepad32/EspBle types leak here.
class ClassicHidHost {
public:
    virtual ~ClassicHidHost() = default;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool startPairing(uint32_t timeoutMs) = 0;
    virtual bool forgetDevice() = 0;
    virtual bool reconnect() = 0;
    virtual ClassicHostState getState() const = 0;
};

enum class BlePeripheralState : uint8_t {
    Idle = 0,
    Advertising,
    Connected,
    Error
};

class BleHidPeripheral {
public:
    virtual ~BleHidPeripheral() = default;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool startPairing(uint32_t timeoutMs) = 0;
    virtual bool forgetHost() = 0;
    virtual BlePeripheralState getState() const = 0;
    virtual bool sendKeyboard(const KeyboardState &state) = 0;
    virtual bool sendMouse(const MouseState &state) = 0;
    virtual bool sendGamepad(const GamepadState &state) = 0;
};

// Null stubs for compile/link without a radio stack (Milestone 2).
class NullClassicHidHost final : public ClassicHidHost {
public:
    bool start() override {
        return true;
    }
    void stop() override {}
    bool startPairing(uint32_t) override {
        state_ = ClassicHostState::Pairing;
        return true;
    }
    bool forgetDevice() override {
        state_ = ClassicHostState::Idle;
        return true;
    }
    bool reconnect() override {
        return false;
    }
    ClassicHostState getState() const override {
        return state_;
    }

private:
    ClassicHostState state_ = ClassicHostState::Idle;
};

class NullBleHidPeripheral final : public BleHidPeripheral {
public:
    bool start() override {
        return true;
    }
    void stop() override {}
    bool startPairing(uint32_t) override {
        state_ = BlePeripheralState::Advertising;
        return true;
    }
    bool forgetHost() override {
        state_ = BlePeripheralState::Idle;
        return true;
    }
    BlePeripheralState getState() const override {
        return state_;
    }
    bool sendKeyboard(const KeyboardState &) override {
        return state_ == BlePeripheralState::Connected;
    }
    bool sendMouse(const MouseState &) override {
        return state_ == BlePeripheralState::Connected;
    }
    bool sendGamepad(const GamepadState &) override {
        return state_ == BlePeripheralState::Connected;
    }

    void mockConnect() {
        state_ = BlePeripheralState::Connected;
    }

private:
    BlePeripheralState state_ = BlePeripheralState::Idle;
};

} // namespace blueshift
