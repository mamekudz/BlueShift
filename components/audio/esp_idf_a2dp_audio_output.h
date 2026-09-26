#pragma once

// Thin on-target adapter stub. Full Bluedroid wiring lives in ESP-IDF builds.
// Host tests use CapturingAudioOutput instead.

#include "audio/a2dp_audio_output.h"

namespace blueshift {
namespace audio {

// Production path: ESP-IDF A2DP Source + in-stack SBC.
// Status: INTERFACE READY / LINK spike proven separately — PHYSICALLY UNVERIFIED.
class EspIdfA2dpAudioOutput final : public A2dpAudioOutput {
public:
    bool begin() override;
    void end() override;
    bool startPairing() override;
    bool connectKnown() override;
    void disconnect() override;
    bool startStream() override;
    void stopStream() override;
    std::size_t supplyPcm(const int16_t *samples, std::size_t count) override;
    std::size_t pullPcm(int16_t *buf, std::size_t maxSamples) override;
    AudioPeerState state() const override;
    uint32_t sampleRateHz() const override;
    void setSampleRateHz(uint32_t hz) override;
    uint32_t underrunCount() const override;
    uint32_t overrunCount() const override;
    uint32_t pcmSamplesSupplied() const override;
    uint32_t pcmSamplesPulled() const override;
    std::size_t pcmQueueDepth() const override;

private:
    AudioPeerState state_ = AudioPeerState::AudioOff;
    uint32_t sampleRateHz_ = 44100;
    uint32_t underruns_ = 0;
    uint32_t overruns_ = 0;
    uint32_t supplied_ = 0;
    uint32_t pulled_ = 0;
    static constexpr std::size_t kPcmCap = 4096;
    int16_t pcm_[kPcmCap]{};
    std::size_t pcmHead_ = 0;
    std::size_t pcmSize_ = 0;
};

} // namespace audio
} // namespace blueshift
