#pragma once

#include <cstdint>

namespace blueshift {

// Project-owned Bluetooth errors — UI maps via i18x; no raw esp_err_t in UI.
enum class BluetoothError : uint8_t {
    None = 0,
    NvsInitFailed,
    ControllerInitFailed,
    ControllerEnableFailed,
    BluedroidInitFailed,
    BluedroidEnableFailed,
    ClassicHostInitFailed,
    BleGapInitFailed,
    BleHidInitFailed,
    AlreadyStarted,
    NotStarted,
    UnsupportedBuild,
    Unknown
};

const char *bluetoothErrorName(BluetoothError err);

} // namespace blueshift
