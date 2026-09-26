#include "audio/esp_idf_a2dp_audio_output.h"

namespace blueshift {
namespace audio {

bool EspIdfA2dpAudioOutput::begin() {
    state_ = AudioPeerState::AudioOff;
    pcmHead_ = 0;
    pcmSize_ = 0;
    // Real init: esp_a2d_source_init + register data_cb — see spike env.
    // Optional until BLUESHIFT_AUDIO_A2DP=1 on device.
    return true;
}

void EspIdfA2dpAudioOutput::end() {
    stopStream();
    state_ = AudioPeerState::AudioOff;
}

bool EspIdfA2dpAudioOutput::startPairing() {
    state_ = AudioPeerState::AudioPairing;
    return true;
}

bool EspIdfA2dpAudioOutput::connectKnown() {
    state_ = AudioPeerState::AudioConnecting;
    // Stack connect async → Connected on callback.
    state_ = AudioPeerState::AudioConnected;
    return true;
}

void EspIdfA2dpAudioOutput::disconnect() {
    state_ = AudioPeerState::AudioLost;
    pcmSize_ = 0;
}

bool EspIdfA2dpAudioOutput::startStream() {
    if (state_ != AudioPeerState::AudioConnected && state_ != AudioPeerState::AudioStreaming) {
        return false;
    }
    state_ = AudioPeerState::AudioStreaming;
    return true;
}

void EspIdfA2dpAudioOutput::stopStream() {
    if (state_ == AudioPeerState::AudioStreaming) {
        state_ = AudioPeerState::AudioConnected;
    }
}

std::size_t EspIdfA2dpAudioOutput::supplyPcm(const int16_t *samples, std::size_t count) {
    if (samples == nullptr || count == 0) {
        return 0;
    }
    std::size_t accepted = 0;
    for (std::size_t i = 0; i < count; ++i) {
        if (pcmSize_ >= kPcmCap) {
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

std::size_t EspIdfA2dpAudioOutput::pullPcm(int16_t *buf, std::size_t maxSamples) {
    // Invoked from A2DP source data callback — keep cheap.
    if (buf == nullptr || maxSamples == 0) {
        return 0;
    }
    for (std::size_t i = 0; i < maxSamples; ++i) {
        if (pcmSize_ == 0) {
            buf[i] = 0;
            ++underruns_;
        } else {
            buf[i] = pcm_[pcmHead_];
            pcmHead_ = (pcmHead_ + 1) % kPcmCap;
            --pcmSize_;
        }
        ++pulled_;
    }
    return maxSamples;
}

AudioPeerState EspIdfA2dpAudioOutput::state() const {
    return state_;
}
uint32_t EspIdfA2dpAudioOutput::sampleRateHz() const {
    return sampleRateHz_;
}
void EspIdfA2dpAudioOutput::setSampleRateHz(uint32_t hz) {
    sampleRateHz_ = hz == 0 ? 44100 : hz;
}
uint32_t EspIdfA2dpAudioOutput::underrunCount() const {
    return underruns_;
}
uint32_t EspIdfA2dpAudioOutput::overrunCount() const {
    return overruns_;
}
uint32_t EspIdfA2dpAudioOutput::pcmSamplesSupplied() const {
    return supplied_;
}
uint32_t EspIdfA2dpAudioOutput::pcmSamplesPulled() const {
    return pulled_;
}

std::size_t EspIdfA2dpAudioOutput::pcmQueueDepth() const {
    return pcmSize_;
}

} // namespace audio
} // namespace blueshift
