#pragma once

#include <cstddef>
#include <cstdint>

#include "audio/speaker_edge.h"

namespace blueshift {
namespace audio {

// Project-owned A2DP Source façade — no raw ESP-IDF callbacks leak upward.
// SBC encoding happens inside ESP-IDF Bluedroid when enabled on-target.
class A2dpAudioOutput {
public:
    virtual ~A2dpAudioOutput() = default;

    virtual bool begin() = 0;
    virtual void end() = 0;

    virtual bool startPairing() = 0;
    virtual bool connectKnown() = 0;
    virtual void disconnect() = 0;

    virtual bool startStream() = 0;
    virtual void stopStream() = 0;

    // Called from audio worker (NOT BLE callback): enqueue PCM for A2DP pull.
    // Returns samples accepted (may be less on overflow → silence later).
    virtual std::size_t supplyPcm(const int16_t *samples, std::size_t count) = 0;

    // A2DP data callback path: fill buf with up to maxSamples of s16 mono.
    // Underrun → write silence, increment underruns; never block HID.
    virtual std::size_t pullPcm(int16_t *buf, std::size_t maxSamples) = 0;

    virtual AudioPeerState state() const = 0;
    virtual uint32_t sampleRateHz() const = 0;
    virtual void setSampleRateHz(uint32_t hz) = 0;

    virtual uint32_t underrunCount() const = 0;
    virtual uint32_t overrunCount() const = 0;
    virtual uint32_t pcmSamplesSupplied() const = 0;
    virtual uint32_t pcmSamplesPulled() const = 0;
    virtual std::size_t pcmQueueDepth() const = 0;

    virtual bool isStreaming() const {
        return state() == AudioPeerState::AudioStreaming;
    }
};

// Host-/unit-testable ring that captures PCM (no Bluetooth).
class CapturingAudioOutput final : public A2dpAudioOutput {
public:
    static constexpr std::size_t kPcmCap = 8192;

    bool begin() override {
        resetStats();
        state_ = AudioPeerState::AudioConnected;
        return true;
    }
    void end() override {
        state_ = AudioPeerState::AudioOff;
        stopStream();
    }
    bool startPairing() override {
        state_ = AudioPeerState::AudioPairing;
        return true;
    }
    bool connectKnown() override {
        state_ = AudioPeerState::AudioConnected;
        return true;
    }
    void disconnect() override {
        state_ = AudioPeerState::AudioLost;
        stopStream();
    }
    bool startStream() override {
        if (state_ == AudioPeerState::AudioOff || state_ == AudioPeerState::AudioError) {
            return false;
        }
        state_ = AudioPeerState::AudioStreaming;
        return true;
    }
    void stopStream() override {
        if (state_ == AudioPeerState::AudioStreaming) {
            state_ = AudioPeerState::AudioConnected;
        }
    }

    std::size_t supplyPcm(const int16_t *samples, std::size_t count) override {
        if (samples == nullptr || count == 0) {
            return 0;
        }
        std::size_t accepted = 0;
        for (std::size_t i = 0; i < count; ++i) {
            if (pcmSize_ >= kPcmCap) {
                // Prefer current audio: drop oldest.
                pcmHead_ = (pcmHead_ + 1) % kPcmCap;
                --pcmSize_;
                ++overruns_;
            }
            pcm_[(pcmHead_ + pcmSize_) % kPcmCap] = samples[i];
            ++pcmSize_;
            ++accepted;
            ++supplied_;
        }
        return accepted;
    }

    std::size_t pullPcm(int16_t *buf, std::size_t maxSamples) override {
        if (buf == nullptr || maxSamples == 0) {
            return 0;
        }
        std::size_t n = 0;
        while (n < maxSamples) {
            if (pcmSize_ == 0) {
                buf[n++] = 0;
                ++underruns_;
                ++pulled_;
                continue;
            }
            buf[n++] = pcm_[pcmHead_];
            pcmHead_ = (pcmHead_ + 1) % kPcmCap;
            --pcmSize_;
            ++pulled_;
        }
        return n;
    }

    AudioPeerState state() const override {
        return state_;
    }
    uint32_t sampleRateHz() const override {
        return sampleRateHz_;
    }
    void setSampleRateHz(uint32_t hz) override {
        sampleRateHz_ = hz == 0 ? 44100 : hz;
    }
    uint32_t underrunCount() const override {
        return underruns_;
    }
    uint32_t overrunCount() const override {
        return overruns_;
    }
    uint32_t pcmSamplesSupplied() const override {
        return supplied_;
    }
    uint32_t pcmSamplesPulled() const override {
        return pulled_;
    }

    std::size_t pcmQueueDepth() const override {
        return pcmSize_;
    }

    // Snapshot capture buffer for tests (copies up to max).
    std::size_t copyCaptured(int16_t *dst, std::size_t max) const {
        std::size_t n = pcmSize_ < max ? pcmSize_ : max;
        for (std::size_t i = 0; i < n; ++i) {
            dst[i] = pcm_[(pcmHead_ + i) % kPcmCap];
        }
        return n;
    }

    void resetStats() {
        pcmHead_ = 0;
        pcmSize_ = 0;
        underruns_ = 0;
        overruns_ = 0;
        supplied_ = 0;
        pulled_ = 0;
    }

private:
    AudioPeerState state_ = AudioPeerState::AudioOff;
    uint32_t sampleRateHz_ = 44100;
    int16_t pcm_[kPcmCap]{};
    std::size_t pcmHead_ = 0;
    std::size_t pcmSize_ = 0;
    uint32_t underruns_ = 0;
    uint32_t overruns_ = 0;
    uint32_t supplied_ = 0;
    uint32_t pulled_ = 0;
};

} // namespace audio
} // namespace blueshift
