#include "bluetooth/bluetooth_error.h"

namespace blueshift {

const char *bluetoothErrorName(BluetoothError err) {
    switch (err) {
    case BluetoothError::None:
        return "None";
    case BluetoothError::NvsInitFailed:
        return "NvsInitFailed";
    case BluetoothError::ControllerInitFailed:
        return "ControllerInitFailed";
    case BluetoothError::ControllerEnableFailed:
        return "ControllerEnableFailed";
    case BluetoothError::BluedroidInitFailed:
        return "BluedroidInitFailed";
    case BluetoothError::BluedroidEnableFailed:
        return "BluedroidEnableFailed";
    case BluetoothError::ClassicHostInitFailed:
        return "ClassicHostInitFailed";
    case BluetoothError::BleGapInitFailed:
        return "BleGapInitFailed";
    case BluetoothError::BleHidInitFailed:
        return "BleHidInitFailed";
    case BluetoothError::AlreadyStarted:
        return "AlreadyStarted";
    case BluetoothError::NotStarted:
        return "NotStarted";
    case BluetoothError::UnsupportedBuild:
        return "UnsupportedBuild";
    case BluetoothError::Unknown:
    default:
        return "Unknown";
    }
}

} // namespace blueshift
