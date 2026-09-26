#include "storage/device_store.h"

#include <cstring>

#if defined(ARDUINO)
#include <Preferences.h>
#elif defined(ESP_PLATFORM)
#include "nvs.h"
#include "nvs_flash.h"
#endif

namespace blueshift {
namespace {

constexpr const char *kNs = "bs_dev";
constexpr const char *kKeyBlob = "dev_v1";

} // namespace

bool DeviceStore::begin() {
#if defined(ARDUINO)
    Preferences prefs;
    ready_ = prefs.begin(kNs, false);
    prefs.end();
    return ready_;
#elif defined(ESP_PLATFORM)
    nvs_handle_t handle = 0;
    if (nvs_open(kNs, NVS_READWRITE, &handle) != ESP_OK) {
        ready_ = false;
        return false;
    }
    nvs_close(handle);
    ready_ = true;
    return true;
#else
    ready_ = true;
    memory_ = makeEmptyDeviceStore();
    return true;
#endif
}

bool DeviceStore::load(DeviceStoreBlob &out) {
    if (!ready_) {
        out = makeEmptyDeviceStore();
        return false;
    }
#if defined(ARDUINO)
    Preferences prefs;
    if (!prefs.begin(kNs, true)) {
        out = makeEmptyDeviceStore();
        return false;
    }
    DeviceStoreBlob raw = makeEmptyDeviceStore();
    const size_t n = prefs.getBytes(kKeyBlob, &raw, sizeof(raw));
    prefs.end();
    if (n != sizeof(raw) || !validateDeviceStore(raw)) {
        out = makeEmptyDeviceStore();
        return false;
    }
    out = raw;
    memory_ = raw;
    return true;
#elif defined(ESP_PLATFORM)
    nvs_handle_t handle = 0;
    if (nvs_open(kNs, NVS_READONLY, &handle) != ESP_OK) {
        out = makeEmptyDeviceStore();
        return false;
    }
    DeviceStoreBlob raw = makeEmptyDeviceStore();
    size_t len = sizeof(raw);
    const esp_err_t err = nvs_get_blob(handle, kKeyBlob, &raw, &len);
    nvs_close(handle);
    if (err != ESP_OK || len != sizeof(raw) || !validateDeviceStore(raw)) {
        out = makeEmptyDeviceStore();
        return false;
    }
    out = raw;
    memory_ = raw;
    return true;
#else
    out = memory_;
    return validateDeviceStore(out);
#endif
}

bool DeviceStore::save(const DeviceStoreBlob &blob) {
    if (!ready_ || !validateDeviceStore(blob)) {
        return false;
    }
#if defined(ARDUINO)
    Preferences prefs;
    if (!prefs.begin(kNs, false)) {
        return false;
    }
    const size_t n = prefs.putBytes(kKeyBlob, &blob, sizeof(blob));
    prefs.end();
    if (n == sizeof(blob)) {
        memory_ = blob;
        return true;
    }
    return false;
#elif defined(ESP_PLATFORM)
    nvs_handle_t handle = 0;
    if (nvs_open(kNs, NVS_READWRITE, &handle) != ESP_OK) {
        return false;
    }
    const esp_err_t err = nvs_set_blob(handle, kKeyBlob, &blob, sizeof(blob));
    if (err == ESP_OK) {
        nvs_commit(handle);
        memory_ = blob;
    }
    nvs_close(handle);
    return err == ESP_OK;
#else
    memory_ = blob;
    return true;
#endif
}

bool DeviceStore::clear() {
#if defined(ARDUINO)
    Preferences prefs;
    if (!prefs.begin(kNs, false)) {
        return false;
    }
    prefs.clear();
    prefs.end();
    memory_ = makeEmptyDeviceStore();
    return true;
#elif defined(ESP_PLATFORM)
    nvs_handle_t handle = 0;
    if (nvs_open(kNs, NVS_READWRITE, &handle) != ESP_OK) {
        return false;
    }
    nvs_erase_all(handle);
    nvs_commit(handle);
    nvs_close(handle);
    memory_ = makeEmptyDeviceStore();
    return true;
#else
    memory_ = makeEmptyDeviceStore();
    return true;
#endif
}

bool DeviceStore::upsert(const KnownDeviceMeta &meta) {
    if (!meta.used || meta.friendlyName[0] == '\0') {
        return false;
    }
    DeviceStoreBlob blob = memory_;
    int freeSlot = -1;
    for (uint8_t i = 0; i < kMaxKnownDevices; ++i) {
        auto &slot = blob.devices[i];
        if (slot.used && slot.kind == meta.kind &&
            std::strncmp(slot.friendlyName, meta.friendlyName, kDeviceNameMax) == 0) {
            slot = meta;
            return save(blob);
        }
        if (!slot.used && freeSlot < 0) {
            freeSlot = static_cast<int>(i);
        }
    }
    if (freeSlot < 0) {
        return false;
    }
    blob.devices[freeSlot] = meta;
    return save(blob);
}

bool DeviceStore::forget(KnownDeviceKind kind, const char *friendlyName) {
    if (friendlyName == nullptr || friendlyName[0] == '\0') {
        return false;
    }
    DeviceStoreBlob blob = memory_;
    bool found = false;
    for (auto &slot : blob.devices) {
        if (slot.used && slot.kind == kind &&
            std::strncmp(slot.friendlyName, friendlyName, kDeviceNameMax) == 0) {
            slot = KnownDeviceMeta{};
            found = true;
        }
    }
    return found && save(blob);
}

const KnownDeviceMeta *DeviceStore::findClassicInput() const {
    for (const auto &slot : memory_.devices) {
        if (slot.used && slot.kind == KnownDeviceKind::ClassicInput) {
            return &slot;
        }
    }
    return nullptr;
}

} // namespace blueshift
