#pragma once

#include "blueshift/evidence.h"
#include "hardware/t_lion/board_pins.h"

namespace blueshift {
namespace t_lion {

// Single board facade — application code must not hard-code GPIOs.

struct BoardConfig {
    Evidence evidence = Evidence::Documented;
    int oledSda = kOledSda;
    int oledScl = kOledScl;
    uint8_t oledAddress = kOledI2cAddress;
    int oledWidth = kOledWidth;
    int oledHeight = kOledHeight;
    int btnUp = kBtnUp;
    int btnDown = kBtnDown;
    int btnConfirm = kBtnConfirm;
    int btnLeft = kBtnLeft;
    int btnRight = kBtnRight;
    int batteryAdc = kBatteryAdc;
    float batteryDivider = kBatteryDividerRatio;
    int statusLed = kStatusLed;
};

inline BoardConfig makeBoardConfig() {
    return BoardConfig{};
}

// Electrical notes (DOCUMENTED / ASSUMED):
// - GPIO34/36/39 are input-only: no internal pull-up. Rely on board resistors.
// - GPIO32/33 may use INPUT_PULLUP if board has none (ASSUMED board has pulls).
// - Charging status pin to MCU: UNKNOWN (TP5400 CHRG may drive LED only).

} // namespace t_lion
} // namespace blueshift
