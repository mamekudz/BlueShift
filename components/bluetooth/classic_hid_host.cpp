#include "bluetooth/classic_hid_host.h"

#include "blueshift/log.h"

namespace blueshift {

bool ClassicHidHostSpike::start() {
    state_ = ClassicHostState::Idle;
    scanning_ = false;
    pairing_ = false;
    BS_LOG_INFO("BT-CLASSIC", "spike start (no Classic radio linked — stub)");
    return true;
}

void ClassicHidHostSpike::stop() {
    scanning_ = false;
    pairing_ = false;
    if (state_ == ClassicHostState::Connected) {
        state_ = ClassicHostState::Idle;
    }
}

bool ClassicHidHostSpike::startScan(uint32_t timeoutMs) {
    if (timeoutMs == 0) {
        timeoutMs = scanTimeoutMs_;
    }
    scanTimeoutMs_ = timeoutMs;
    scanning_ = true;
    pairing_ = false;
    deadlineMs_ = 0; // set on first tick with now
    state_ = ClassicHostState::Pairing; // scan/pair umbrella for UI
    BS_LOG_INFO("BT-CLASSIC", "scan start timeout=%lu ms", static_cast<unsigned long>(timeoutMs));
    return true;
}

bool ClassicHidHostSpike::startPairing(uint32_t timeoutMs) {
    if (timeoutMs == 0) {
        timeoutMs = pairTimeoutMs_;
    }
    pairTimeoutMs_ = timeoutMs;
    pairing_ = true;
    scanning_ = false;
    deadlineMs_ = 0;
    state_ = ClassicHostState::Pairing;
    BS_LOG_INFO("BT-CLASSIC", "pair start timeout=%lu ms", static_cast<unsigned long>(timeoutMs));
    return true;
}

bool ClassicHidHostSpike::connect() {
    // Without radio: allow simulation path to mark connected.
    state_ = ClassicHostState::Connected;
    everConnected_ = true;
    scanning_ = false;
    pairing_ = false;
    return true;
}

bool ClassicHidHostSpike::disconnect() {
    if (state_ == ClassicHostState::Connected) {
        state_ = ClassicHostState::Idle;
    }
    return true;
}

bool ClassicHidHostSpike::forgetDevice() {
    everConnected_ = false;
    state_ = ClassicHostState::Idle;
    scanning_ = false;
    pairing_ = false;
    return true;
}

bool ClassicHidHostSpike::reconnect() {
    if (!everConnected_) {
        return false;
    }
    state_ = ClassicHostState::Reconnecting;
    // Stub: immediate reconnect success for simulation hooks.
    state_ = ClassicHostState::Connected;
    return true;
}

void ClassicHidHostSpike::tick(uint32_t nowMs) {
    if (!scanning_ && !pairing_) {
        return;
    }
    if (deadlineMs_ == 0) {
        deadlineMs_ = nowMs + (scanning_ ? scanTimeoutMs_ : pairTimeoutMs_);
        return;
    }
    if (nowMs >= deadlineMs_) {
        BS_LOG_WARN("BT-CLASSIC", "pairing/scan timeout");
        scanning_ = false;
        pairing_ = false;
        if (state_ == ClassicHostState::Pairing) {
            state_ = ClassicHostState::Idle;
        }
    }
}

void ClassicHidHostSpike::injectRaw(const RawHidReport &report) {
    if (cb_ != nullptr) {
        cb_(report, user_);
    }
}

} // namespace blueshift
