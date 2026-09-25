#include "battery/adc_battery.h"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

namespace blueshift {

AdcBatteryBackend::AdcBatteryBackend(const t_lion::BoardConfig &cfg) : cfg_(cfg) {}

bool AdcBatteryBackend::begin() {
#if defined(ARDUINO)
    analogReadResolution(12);
    ready_ = true;
    return true;
#else
    ready_ = false;
    return false;
#endif
}

AdcReading AdcBatteryBackend::readRaw() {
    AdcReading r;
#if defined(ARDUINO)
    if (!ready_) {
        return r;
    }
    r.raw = analogRead(cfg_.batteryAdc);
    // DOCUMENTED LilyGO formula: (raw/4095)*divider*3.3*(vref/1000)
    r.millivolts =
        (static_cast<float>(r.raw) / 4095.0f) * cfg_.batteryDivider * 3300.0f * (vrefMv_ / 1000.0f) *
        calGain_;
    r.valid = r.raw >= 0 && r.raw <= 4095;
#else
    r.valid = false;
#endif
    return r;
}

BatterySample AdcBatteryBackend::sample() {
    BatterySample s;
    const AdcReading r = readRaw();
    s.valid = r.valid;
    s.voltageMv = r.millivolts;
    // Charging / USB detection: UNKNOWN on this board without verified CHRG GPIO.
    s.charging = false;
    s.externalPower = false;
    return s;
}

} // namespace blueshift
