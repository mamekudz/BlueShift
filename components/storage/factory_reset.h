#pragma once

#include "bluetooth/hid_transport.h"
#include "storage/config_store.h"

namespace blueshift {

struct FactoryResetHooks {
    ClassicHidHost *classic = nullptr;
    BleHidPeripheral *ble = nullptr;
    ConfigStore *config = nullptr;
};

struct FactoryResetResult {
    bool settingsCleared = false;
    bool classicForgetOk = false;
    bool bleForgetOk = false;
    bool ok = false;
};

// Orchestrates controlled cleanup — does not wipe entire NVS partition.
class FactoryReset {
public:
    static FactoryResetResult run(const FactoryResetHooks &hooks);
};

} // namespace blueshift
