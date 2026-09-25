#include "battery/battery_monitor.h"

#include <cstddef>

namespace blueshift {

uint8_t BatteryMonitor::percentFromVoltageMv(float voltageMv) {
    // Approximate single-cell Li-ion discharge curve (resting-ish).
    // Points are ASSUMED pending physical calibration.
    struct Point {
        float mv;
        uint8_t pct;
    };
    static const Point kCurve[] = {
        {4200.0f, 100}, {4100.0f, 90}, {4000.0f, 80}, {3900.0f, 70},
        {3800.0f, 55},  {3700.0f, 40}, {3600.0f, 25}, {3500.0f, 15},
        {3400.0f, 8},   {3300.0f, 3},  {3200.0f, 0},
    };

    if (voltageMv >= kCurve[0].mv) {
        return 100;
    }
    const auto n = sizeof(kCurve) / sizeof(kCurve[0]);
    if (voltageMv <= kCurve[n - 1].mv) {
        return 0;
    }
    for (std::size_t i = 0; i + 1 < n; ++i) {
        if (voltageMv <= kCurve[i].mv && voltageMv >= kCurve[i + 1].mv) {
            const float span = kCurve[i].mv - kCurve[i + 1].mv;
            const float t = span > 0 ? (kCurve[i].mv - voltageMv) / span : 0;
            const float pct =
                kCurve[i].pct + t * (static_cast<float>(kCurve[i + 1].pct) - kCurve[i].pct);
            if (pct <= 0) {
                return 0;
            }
            if (pct >= 100) {
                return 100;
            }
            return static_cast<uint8_t>(pct + 0.5f);
        }
    }
    return 0xFF;
}

BatteryStatus BatteryMonitor::update(const BatterySample &sample) {
    BatteryStatus st;
    if (!sample.valid) {
        return st;
    }
    st.valid = true;
    st.voltageMv = sample.voltageMv;
    st.charging = sample.charging;
    st.externalPower = sample.externalPower;
    st.percent = percentFromVoltageMv(sample.voltageMv);

    if (sample.charging) {
        st.health = BatteryHealth::Charging;
    } else if (sample.externalPower) {
        st.health = BatteryHealth::ExternalPower;
    } else if (sample.voltageMv <= criticalMv_) {
        st.health = BatteryHealth::Critical;
    } else if (sample.voltageMv <= lowMv_) {
        st.health = BatteryHealth::Low;
    } else {
        st.health = BatteryHealth::Ok;
    }
    return st;
}

} // namespace blueshift
