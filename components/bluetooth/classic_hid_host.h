#pragma once

#include <cstdint>

#include "bluetooth/hid_transport.h"
#include "hid/device_parsers.h"

namespace blueshift {

using ClassicRawHidFn = void (*)(const RawHidReport &report, void *user);

// Classic HID Host spike — compiles for T-Lion.
// Radio stack: deferred (Arduino 2.x + NimBLE dual-host conflict; see docs).
// IMPLEMENTED_UNVERIFIED state machine + raw callback path.
class ClassicHidHostSpike : public ClassicHidHost {
public:
    void setRawCallback(ClassicRawHidFn fn, void *user) {
        cb_ = fn;
        user_ = user;
    }

    bool start() override;
    void stop() override;
    bool startPairing(uint32_t timeoutMs) override;
    bool forgetDevice() override;
    bool reconnect() override;
    ClassicHostState getState() const override {
        return state_;
    }

    bool startScan(uint32_t timeoutMs);
    bool connect();
    bool disconnect();
    void tick(uint32_t nowMs);

    void injectRaw(const RawHidReport &report);

    void setStateForTest(ClassicHostState s) {
        state_ = s;
    }

private:
    ClassicHostState state_ = ClassicHostState::Idle;
    ClassicRawHidFn cb_ = nullptr;
    void *user_ = nullptr;
    uint32_t scanTimeoutMs_ = 60000;
    uint32_t pairTimeoutMs_ = 60000;
    uint32_t deadlineMs_ = 0;
    bool scanning_ = false;
    bool pairing_ = false;
    bool everConnected_ = false;
};

using MockClassicHidHost = ClassicHidHostSpike;

} // namespace blueshift
