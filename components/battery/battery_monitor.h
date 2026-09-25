#pragma once

#include <cstdint>

namespace blueshift {

enum class BatteryHealth : uint8_t {
    Unknown = 0,
    Ok,
    Low,
    Critical,
    Charging,
    ExternalPower
};

struct BatterySample {
    float voltageMv = 0;
    bool charging = false;
    bool externalPower = false;
    bool valid = false;
};

struct BatteryStatus {
    float voltageMv = 0;
    uint8_t percent = 0xFF; // 0xFF unknown
    BatteryHealth health = BatteryHealth::Unknown;
    bool charging = false;
    bool externalPower = false;
    bool valid = false;
};

// Piecewise Li-ion open-circuit style curve (approximate). Not linear with V.
class BatteryMonitor {
public:
    void setThresholds(float lowMv, float criticalMv) {
        lowMv_ = lowMv;
        criticalMv_ = criticalMv;
    }

    BatteryStatus update(const BatterySample &sample);

    static uint8_t percentFromVoltageMv(float voltageMv);

private:
    float lowMv_ = 3500.0f;
    float criticalMv_ = 3300.0f;
};

} // namespace blueshift
