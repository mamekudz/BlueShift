#pragma once

#include <cstdint>
#include <cstring>

#include "bluetooth/device_profiles.h"
#include "storage/app_config.h"

namespace blueshift {

// App-owned device metadata — separate from Bluedroid bond material.
// Do not store unnecessary personal identifiers.
inline constexpr uint8_t kMaxKnownDevices = 4;
inline constexpr uint8_t kDeviceNameMax = 24;

enum class KnownDeviceKind : uint8_t {
    ClassicInput = 0,
    BleHost = 1
};

struct KnownDeviceMeta {
    bool used = false;
    KnownDeviceKind kind = KnownDeviceKind::ClassicInput;
    char friendlyName[kDeviceNameMax] = {};
    char profileId[32] = {}; // e.g. profiles::kSn30Pro.id
    OutputMode preferredOutput = OutputMode::Auto;
    uint32_t lastConnectedUnix = 0; // 0 = unknown / not set
    uint8_t bdAddr[6] = {};         // optional; zeroed if unknown
    bool hasBdAddr = false;
};

struct DeviceStoreBlob {
    uint16_t schemaVersion = 1;
    KnownDeviceMeta devices[kMaxKnownDevices]{};
};

inline DeviceStoreBlob makeEmptyDeviceStore() {
    return DeviceStoreBlob{};
}

inline bool validateDeviceStore(const DeviceStoreBlob &blob) {
    if (blob.schemaVersion == 0 || blob.schemaVersion > 1) {
        return false;
    }
    for (const auto &d : blob.devices) {
        if (!d.used) {
            continue;
        }
        if (d.friendlyName[0] == '\0') {
            return false;
        }
        if (d.preferredOutput > OutputMode::Gamepad) {
            return false;
        }
    }
    return true;
}

// NVS namespace "bs_dev" — separate from "blueshift" settings and BT bonds.
class DeviceStore {
public:
    bool begin();
    bool load(DeviceStoreBlob &out);
    bool save(const DeviceStoreBlob &blob);
    bool clear();

    // Upsert by friendly name + kind (host-safe; physical BDADDR later).
    bool upsert(const KnownDeviceMeta &meta);
    bool forget(KnownDeviceKind kind, const char *friendlyName);
    const KnownDeviceMeta *findClassicInput() const;
    const DeviceStoreBlob &blob() const {
        return memory_;
    }

private:
    bool ready_ = false;
    DeviceStoreBlob memory_{};
};

} // namespace blueshift
