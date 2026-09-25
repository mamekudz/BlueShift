#include "power/power_policy.h"

namespace blueshift {

const char *powerStateName(PowerState state) {
    switch (state) {
    case PowerState::Boot:
        return "BOOT";
    case PowerState::Active:
        return "ACTIVE";
    case PowerState::Idle:
        return "IDLE";
    case PowerState::Pairing:
        return "PAIRING";
    case PowerState::LowBattery:
        return "LOW_BATTERY";
    case PowerState::CriticalBattery:
        return "CRITICAL_BATTERY";
    case PowerState::UsbPowered:
        return "USB_POWERED";
    case PowerState::Sleep:
        return "SLEEP";
    }
    return "UNKNOWN";
}

} // namespace blueshift
