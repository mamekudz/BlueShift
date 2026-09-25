#pragma once

#include "blueshift/log.h"
#include "storage/app_config.h"

namespace blueshift {

// Extended fields for Milestone 3 persistence.
struct AppConfigV1 {
    AppConfig base{};
    LogLevel logLevel = LogLevel::Info;
    uint16_t classicScanTimeoutSec = 60;
    uint16_t classicPairTimeoutSec = 60;
    uint16_t blePairTimeoutSec = 60;
    char bleKeyboardName[24] = "BlueShift Keyboard";
    char bleGamepadName[24] = "BlueShift Gamepad";
};

inline AppConfigV1 makeDefaultConfigV1() {
    return AppConfigV1{};
}

inline bool validateConfigV1(const AppConfigV1 &cfg) {
    if (!validateConfig(cfg.base)) {
        return false;
    }
    if (cfg.classicScanTimeoutSec == 0 || cfg.classicScanTimeoutSec > 600) {
        return false;
    }
    if (cfg.classicPairTimeoutSec == 0 || cfg.classicPairTimeoutSec > 600) {
        return false;
    }
    if (cfg.blePairTimeoutSec == 0 || cfg.blePairTimeoutSec > 600) {
        return false;
    }
    if (cfg.bleKeyboardName[0] == '\0' || cfg.bleGamepadName[0] == '\0') {
        return false;
    }
    return true;
}

inline AppConfigV1 migrateOrDefaultV1(const AppConfigV1 &raw) {
    if (!validateConfigV1(raw)) {
        return makeDefaultConfigV1();
    }
    AppConfigV1 cfg = raw;
    cfg.base = migrateOrDefault(raw.base);
    return cfg;
}

// NVS-backed store. Host builds use in-memory fallback.
class ConfigStore {
public:
    bool begin();
    bool load(AppConfigV1 &out);
    bool save(const AppConfigV1 &cfg);
    bool clear();

private:
    bool ready_ = false;
    AppConfigV1 memory_{};
};

} // namespace blueshift
