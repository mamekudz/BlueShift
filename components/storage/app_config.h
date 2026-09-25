#pragma once

#include <cstdint>

namespace blueshift {

// Versioned persistent configuration schema (values only — storage backend TBD).

inline constexpr uint16_t kConfigSchemaVersion = 1;

enum class OutputMode : uint8_t {
    Auto = 0,
    Keyboard,
    Mouse,
    Gamepad
};

struct AppConfig {
    uint16_t schemaVersion = kConfigSchemaVersion;
    uint8_t locale = 0; // 0=en-US, 1=de-DE
    uint16_t oledTimeoutSec = 30;
    uint8_t brightness = 255; // if supported later
    uint8_t preferredProfileId = 0;
    OutputMode preferredOutput = OutputMode::Auto;
    bool dirty = false;
};

inline AppConfig makeDefaultConfig() {
    return AppConfig{};
}

inline bool validateConfig(const AppConfig &cfg) {
    if (cfg.schemaVersion == 0 || cfg.schemaVersion > kConfigSchemaVersion) {
        return false;
    }
    if (cfg.locale > 1) {
        return false;
    }
    if (cfg.oledTimeoutSec > 600) {
        return false;
    }
    return true;
}

inline AppConfig migrateOrDefault(const AppConfig &raw) {
    if (!validateConfig(raw)) {
        return makeDefaultConfig();
    }
    AppConfig cfg = raw;
    cfg.schemaVersion = kConfigSchemaVersion;
    cfg.dirty = false;
    return cfg;
}

} // namespace blueshift
