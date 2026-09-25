#pragma once

#include <cstdint>

#include "battery/battery_monitor.h"

namespace blueshift {

enum class PowerState : uint8_t {
    Boot = 0,
    Active,
    Idle,
    Pairing,
    LowBattery,
    CriticalBattery,
    UsbPowered,
    Sleep
};

const char *powerStateName(PowerState state);

class PowerPolicy {
public:
    PowerState state() const {
        return state_;
    }

    void onBootDone() {
        state_ = PowerState::Active;
    }

    void onBattery(const BatteryStatus &bat) {
        if (bat.externalPower || bat.charging) {
            state_ = PowerState::UsbPowered;
            return;
        }
        if (bat.health == BatteryHealth::Critical) {
            state_ = PowerState::CriticalBattery;
            return;
        }
        if (bat.health == BatteryHealth::Low) {
            state_ = PowerState::LowBattery;
            return;
        }
        if (state_ == PowerState::UsbPowered || state_ == PowerState::LowBattery ||
            state_ == PowerState::CriticalBattery) {
            state_ = PowerState::Active;
        }
    }

    void setPairing(bool pairing) {
        if (pairing && state_ != PowerState::CriticalBattery) {
            state_ = PowerState::Pairing;
        } else if (!pairing && state_ == PowerState::Pairing) {
            state_ = PowerState::Active;
        }
    }

    // Aggressive sleep is intentionally not enabled until physical bring-up.
    bool sleepAllowed() const {
        return false;
    }

private:
    PowerState state_ = PowerState::Boot;
};

} // namespace blueshift
