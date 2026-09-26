#pragma once

#include <cstdint>

namespace blueshift {

// Production reconnect policy — bounded retry / backoff.
// Status: IMPLEMENTED_UNVERIFIED (no physical radio yet).

struct ReconnectPolicyConfig {
    uint8_t maxAttempts = 5;
    uint32_t initialDelayMs = 1000;
    uint32_t maxDelayMs = 30000;
    uint32_t pairingTimeoutMs = 60000;
};

class ReconnectPolicy {
public:
    explicit ReconnectPolicy(ReconnectPolicyConfig cfg = {}) : cfg_(cfg) {}

    void reset() {
        attempt_ = 0;
        nextAttemptMs_ = 0;
        exhausted_ = false;
    }

    bool exhausted() const {
        return exhausted_;
    }

    uint8_t attempt() const {
        return attempt_;
    }

    // Returns true when a reconnect attempt should run at nowMs.
    bool shouldAttempt(uint32_t nowMs) {
        if (exhausted_) {
            return false;
        }
        if (attempt_ >= cfg_.maxAttempts) {
            exhausted_ = true;
            return false;
        }
        if (nextAttemptMs_ == 0) {
            nextAttemptMs_ = nowMs;
        }
        return nowMs >= nextAttemptMs_;
    }

    // Call after a failed attempt; schedules next backoff window.
    void recordFailure(uint32_t nowMs) {
        ++attempt_;
        if (attempt_ >= cfg_.maxAttempts) {
            exhausted_ = true;
            return;
        }
        uint32_t delay = cfg_.initialDelayMs;
        for (uint8_t i = 1; i < attempt_; ++i) {
            if (delay >= cfg_.maxDelayMs / 2u) {
                delay = cfg_.maxDelayMs;
                break;
            }
            delay *= 2u;
        }
        if (delay > cfg_.maxDelayMs) {
            delay = cfg_.maxDelayMs;
        }
        nextAttemptMs_ = nowMs + delay;
    }

    void recordSuccess() {
        reset();
    }

    const ReconnectPolicyConfig &config() const {
        return cfg_;
    }

private:
    ReconnectPolicyConfig cfg_{};
    uint8_t attempt_ = 0;
    uint32_t nextAttemptMs_ = 0;
    bool exhausted_ = false;
};

} // namespace blueshift
