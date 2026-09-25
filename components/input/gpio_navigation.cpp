#include "input/gpio_navigation.h"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

namespace blueshift {

GpioNavigationBackend::GpioNavigationBackend(const t_lion::BoardConfig &cfg) : cfg_(cfg) {}

bool GpioNavigationBackend::begin() {
#if defined(ARDUINO)
    // 32/33: internal pull-up OK if board lacks external (ASSUMED board has pulls too).
    pinMode(cfg_.btnUp, INPUT_PULLUP);
    pinMode(cfg_.btnDown, INPUT_PULLUP);
    // 34/36/39: input-only — NO internal pull-up. Board must provide resistors (DOCUMENTED ESP32).
    pinMode(cfg_.btnConfirm, INPUT);
    pinMode(cfg_.btnLeft, INPUT);
    pinMode(cfg_.btnRight, INPUT);
    ready_ = true;
    return true;
#else
    ready_ = false;
    return false;
#endif
}

void GpioNavigationBackend::poll(uint32_t nowMs, NavigationInput &nav) {
#if defined(ARDUINO)
    if (!ready_) {
        return;
    }
    // Active-low (ASSUMED / typical Button2 usage in LilyGO example).
    const bool up = digitalRead(cfg_.btnUp) == LOW;
    const bool down = digitalRead(cfg_.btnDown) == LOW;
    const bool left = digitalRead(cfg_.btnLeft) == LOW;
    const bool right = digitalRead(cfg_.btnRight) == LOW;
    const bool confirm = digitalRead(cfg_.btnConfirm) == LOW;
    nav.setRaw(up, down, left, right, confirm, nowMs);
#else
    (void)nowMs;
    (void)nav;
#endif
}

} // namespace blueshift
