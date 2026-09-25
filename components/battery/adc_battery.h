#pragma once

#include "battery/battery_monitor.h"
#include "hardware/t_lion/board.h"

namespace blueshift {

struct AdcReading {
    int raw = 0;
    float millivolts = 0;
    bool valid = false;
};

// GPIO35 battery ADC — DOCUMENTED divider. Calibration pending physical bring-up.
class AdcBatteryBackend {
public:
    explicit AdcBatteryBackend(const t_lion::BoardConfig &cfg);

    bool begin();
    AdcReading readRaw();
    BatterySample sample();

    void setVrefMv(float vrefMv) {
        vrefMv_ = vrefMv;
    }

    void setCalibrationGain(float gain) {
        calGain_ = gain;
    }

private:
    t_lion::BoardConfig cfg_;
    float vrefMv_ = 1100.0f;
    float calGain_ = 1.0f;
    bool ready_ = false;
};

} // namespace blueshift
