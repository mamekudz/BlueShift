#pragma once

#include "bluetooth/hid_transport.h"
#include "storage/config_store.h"
#include "storage/device_store.h"

namespace blueshift {

enum class FactoryResetScope : uint8_t {
    SettingsOnly = 0,
    ForgetClassic,
    ForgetBle,
    FullBlueShift // settings + device meta + classic forget + ble forget (not full NVS erase)
};

struct FactoryResetHooks {
    ClassicHidHost *classic = nullptr;
    BleHidPeripheral *ble = nullptr;
    ConfigStore *config = nullptr;
    DeviceStore *devices = nullptr;
};

struct FactoryResetResult {
    bool settingsCleared = false;
    bool devicesCleared = false;
    bool classicForgetOk = false;
    bool bleForgetOk = false;
    bool ok = false;
};

// Orchestrates controlled cleanup — does not wipe entire NVS partition.
class FactoryReset {
public:
    static FactoryResetResult run(const FactoryResetHooks &hooks,
                                  FactoryResetScope scope = FactoryResetScope::FullBlueShift);
};

} // namespace blueshift
