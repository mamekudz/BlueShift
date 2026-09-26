#include "storage/config_store.h"

#include <cstring>

#if defined(ARDUINO)
#include <Preferences.h>
#elif defined(ESP_PLATFORM)
#include "nvs.h"
#include "nvs_flash.h"
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
#elif defined(ESP_PLATFORM)
    // App settings namespace — separate from Bluedroid bond storage.
    nvs_handle_t handle = 0;
    const esp_err_t err = nvs_open(kNs, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ready_ = false;
        return false;
    }
    nvs_close(handle);
    ready_ = true;
    return true;
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
#elif defined(ESP_PLATFORM)
    nvs_handle_t handle = 0;
    if (nvs_open(kNs, NVS_READONLY, &handle) != ESP_OK) {
        out = makeDefaultConfigV1();
        return false;
    }
    AppConfigV1 raw = makeDefaultConfigV1();
    size_t len = sizeof(raw);
    const esp_err_t err = nvs_get_blob(handle, kKeyBlob, &raw, &len);
    nvs_close(handle);
    if (err != ESP_OK || len != sizeof(raw)) {
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
#elif defined(ESP_PLATFORM)
    nvs_handle_t handle = 0;
    if (nvs_open(kNs, NVS_READWRITE, &handle) != ESP_OK) {
        return false;
    }
    const esp_err_t err = nvs_set_blob(handle, kKeyBlob, &cfg, sizeof(cfg));
    if (err == ESP_OK) {
        nvs_commit(handle);
    }
    nvs_close(handle);
    return err == ESP_OK;
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
#elif defined(ESP_PLATFORM)
    nvs_handle_t handle = 0;
    if (nvs_open(kNs, NVS_READWRITE, &handle) != ESP_OK) {
        return false;
    }
    nvs_erase_all(handle);
    nvs_commit(handle);
    nvs_close(handle);
    return true;
#else
    memory_ = makeDefaultConfigV1();
    return true;
#endif
}

} // namespace blueshift
