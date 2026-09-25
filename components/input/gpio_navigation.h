#pragma once

#include "hardware/t_lion/board.h"
#include "input/navigation_input.h"

namespace blueshift {

// Reads documented T-Lion 5-way GPIOs into NavigationInput.
// IMPLEMENTED_UNVERIFIED — electrical pulls ASSUMED per board notes.

class GpioNavigationBackend {
public:
    explicit GpioNavigationBackend(const t_lion::BoardConfig &cfg);

    bool begin();
    void poll(uint32_t nowMs, NavigationInput &nav);

private:
    t_lion::BoardConfig cfg_;
    bool ready_ = false;
};

} // namespace blueshift
