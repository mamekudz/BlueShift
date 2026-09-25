#include "storage/config_store.h"

#include <cstring>

#if defined(ARDUINO)
#include <Preferences.h>
#endif

namespace blueshift {
namespace {

constexpr const char *kNs = "blueshift";
constexpr const char *kKeyBlob = "cfg_v1";

} // namespace

bool ConfigStore::begin() {
#if defined(ARDUINO)
    Preferences prefs;
    ready_ = prefs.begin(kNs, false);
    prefs.end();
    return ready_;
#else
    ready_ = true;
    memory_ = makeDefaultConfigV1();
    return true;
#endif
}

bool ConfigStore::load(AppConfigV1 &out) {
    if (!ready_) {
        out = makeDefaultConfigV1();
        return false;
    }
#if defined(ARDUINO)
    Preferences prefs;
    if (!prefs.begin(kNs, true)) {
        out = makeDefaultConfigV1();
        return false;
    }
    AppConfigV1 raw = makeDefaultConfigV1();
    const size_t n = prefs.getBytes(kKeyBlob, &raw, sizeof(raw));
    prefs.end();
    if (n != sizeof(raw)) {
        out = makeDefaultConfigV1();
        return false;
    }
    out = migrateOrDefaultV1(raw);
    return validateConfigV1(out);
#else
    out = migrateOrDefaultV1(memory_);
    return true;
#endif
}

bool ConfigStore::save(const AppConfigV1 &cfg) {
    if (!ready_ || !validateConfigV1(cfg)) {
        return false;
    }
#if defined(ARDUINO)
    Preferences prefs;
    if (!prefs.begin(kNs, false)) {
        return false;
    }
    const size_t n = prefs.putBytes(kKeyBlob, &cfg, sizeof(cfg));
    prefs.end();
    return n == sizeof(cfg);
#else
    memory_ = cfg;
    return true;
#endif
}

bool ConfigStore::clear() {
#if defined(ARDUINO)
    Preferences prefs;
    if (!prefs.begin(kNs, false)) {
        return false;
    }
    prefs.clear();
    prefs.end();
    return true;
#else
    memory_ = makeDefaultConfigV1();
    return true;
#endif
}

} // namespace blueshift
