#include "input/gpio_navigation.h"

#if defined(ARDUINO)
#include <Arduino.h>
#elif defined(ESP_PLATFORM)
#include "driver/gpio.h"
#endif

namespace blueshift {

GpioNavigationBackend::GpioNavigationBackend(const t_lion::BoardConfig &cfg) : cfg_(cfg) {}

bool GpioNavigationBackend::begin() {
#if defined(ARDUINO)
    pinMode(cfg_.btnUp, INPUT_PULLUP);
    pinMode(cfg_.btnDown, INPUT_PULLUP);
    pinMode(cfg_.btnConfirm, INPUT);
    pinMode(cfg_.btnLeft, INPUT);
    pinMode(cfg_.btnRight, INPUT);
    ready_ = true;
    return true;
#elif defined(ESP_PLATFORM)
    // 32/33: internal pull-up OK. 34/36/39: input-only — no internal pull-up (DOCUMENTED).
    gpio_config_t pullCfg = {};
    pullCfg.mode = GPIO_MODE_INPUT;
    pullCfg.pull_up_en = GPIO_PULLUP_ENABLE;
    pullCfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pullCfg.intr_type = GPIO_INTR_DISABLE;
    pullCfg.pin_bit_mask = (1ULL << cfg_.btnUp) | (1ULL << cfg_.btnDown);
    if (gpio_config(&pullCfg) != ESP_OK) {
        return false;
    }

    gpio_config_t floatingCfg = {};
    floatingCfg.mode = GPIO_MODE_INPUT;
    floatingCfg.pull_up_en = GPIO_PULLUP_DISABLE;
    floatingCfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    floatingCfg.intr_type = GPIO_INTR_DISABLE;
    floatingCfg.pin_bit_mask =
        (1ULL << cfg_.btnConfirm) | (1ULL << cfg_.btnLeft) | (1ULL << cfg_.btnRight);
    if (gpio_config(&floatingCfg) != ESP_OK) {
        return false;
    }
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
    const bool up = digitalRead(cfg_.btnUp) == LOW;
    const bool down = digitalRead(cfg_.btnDown) == LOW;
    const bool left = digitalRead(cfg_.btnLeft) == LOW;
    const bool right = digitalRead(cfg_.btnRight) == LOW;
    const bool confirm = digitalRead(cfg_.btnConfirm) == LOW;
    nav.setRaw(up, down, left, right, confirm, nowMs);
#elif defined(ESP_PLATFORM)
    if (!ready_) {
        return;
    }
    const bool up = gpio_get_level(static_cast<gpio_num_t>(cfg_.btnUp)) == 0;
    const bool down = gpio_get_level(static_cast<gpio_num_t>(cfg_.btnDown)) == 0;
    const bool left = gpio_get_level(static_cast<gpio_num_t>(cfg_.btnLeft)) == 0;
    const bool right = gpio_get_level(static_cast<gpio_num_t>(cfg_.btnRight)) == 0;
    const bool confirm = gpio_get_level(static_cast<gpio_num_t>(cfg_.btnConfirm)) == 0;
    nav.setRaw(up, down, left, right, confirm, nowMs);
#else
    (void)nowMs;
    (void)nav;
#endif
}

} // namespace blueshift
