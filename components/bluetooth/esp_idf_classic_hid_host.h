#pragma once

#include "bluetooth/bluetooth_platform.h"
#include "bluetooth/classic_hid_host.h"

namespace blueshift {

// ESP-IDF / Bluedroid Classic HID Host adapter.
// Radio path links only when CONFIG_BT_HID_HOST_ENABLED (not on stock Arduino 2.x SDK).
// IMPLEMENTED_UNVERIFIED.

class EspIdfClassicHidHost final : public ClassicHidHostSpike {
public:
    bool start() override;
    bool startPairing(uint32_t timeoutMs) override;
    bool reconnect() override;

    bool hardwareApiPresent() const {
        return BluetoothPlatform::instance().classicHidHostAvailable();
    }
};

} // namespace blueshift
