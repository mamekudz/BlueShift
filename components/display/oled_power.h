#pragma once

#include "display/display.h"

namespace blueshift {

enum class OledPowerMode : uint8_t {
    Active = 0,
    Dim,
    Off
};

// ACTIVE while interacting; DIM after idle; OFF after timeout.
class OledPowerManager {
public:
    void configure(uint16_t dimAfterSec, uint16_t offAfterSec) {
        dimAfterSec_ = dimAfterSec;
        offAfterSec_ = offAfterSec;
    }

    void notifyActivity(uint32_t nowMs) {
        lastActivityMs_ = nowMs;
        mode_ = OledPowerMode::Active;
    }

    void notifyConnectionChange(uint32_t nowMs) {
        notifyActivity(nowMs);
    }

    OledPowerMode update(uint32_t nowMs) {
        const uint32_t idleMs = nowMs - lastActivityMs_;
        if (offAfterSec_ > 0 && idleMs >= static_cast<uint32_t>(offAfterSec_) * 1000u) {
            mode_ = OledPowerMode::Off;
        } else if (dimAfterSec_ > 0 && idleMs >= static_cast<uint32_t>(dimAfterSec_) * 1000u) {
            mode_ = OledPowerMode::Dim;
        } else {
            mode_ = OledPowerMode::Active;
        }
        return mode_;
    }

    void apply(Display &display) const {
        switch (mode_) {
        case OledPowerMode::Active:
            display.setPower(true);
            display.setContrast(255);
            break;
        case OledPowerMode::Dim:
            display.setPower(true);
            display.setContrast(32);
            break;
        case OledPowerMode::Off:
            display.setPower(false);
            break;
        }
    }

    OledPowerMode mode() const {
        return mode_;
    }

private:
    uint16_t dimAfterSec_ = 15;
    uint16_t offAfterSec_ = 60;
    uint32_t lastActivityMs_ = 0;
    OledPowerMode mode_ = OledPowerMode::Active;
};

} // namespace blueshift
