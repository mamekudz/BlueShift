#pragma once

#include <cstdint>

#include "bluetooth/bluetooth_error.h"
#include "bluetooth/hid_transport.h"

namespace blueshift {

enum class BluetoothPlatformStatus : uint8_t {
    Uninitialized = 0,
    Ready,
    Partial, // e.g. BLE path available, Classic HID Host not in SDK
    Failed
};

struct BluetoothPlatformCapabilities {
    bool controllerBtdm = false;
    bool classicEnabled = false;
    bool bleEnabled = false;
    bool classicHidHostApi = false;
    bool bleHidDeviceApi = false;
};

// Single coherent Bluedroid BTDM bring-up. Do not init NimBLE alongside.
// Classic/BLE adapters must not initialize/deinitialize the controller themselves.
class BluetoothPlatform {
public:
    static BluetoothPlatform &instance();

    BluetoothPlatformCapabilities probeCapabilities() const;
    bool initializeDualMode();
    void deinitialize();

    BluetoothPlatformStatus status() const {
        return status_;
    }

    BluetoothError lastError() const {
        return lastError_;
    }

    const char *statusDetail() const {
        return detail_;
    }

    bool classicHidHostAvailable() const {
        return caps_.classicHidHostApi;
    }

    bool bleHidDeviceAvailable() const {
        return caps_.bleHidDeviceApi;
    }

private:
    BluetoothPlatform() = default;
    bool bluedroidEnabled() const;

    BluetoothPlatformStatus status_ = BluetoothPlatformStatus::Uninitialized;
    BluetoothPlatformCapabilities caps_{};
    BluetoothError lastError_ = BluetoothError::None;
    const char *detail_ = "uninitialized";
    bool started_ = false;
};

} // namespace blueshift
