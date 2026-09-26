#pragma once

#include <cstdint>

namespace blueshift {
namespace audio {

// Transport-independent speaker edge after protocol parse.
// A2DP / BLE must not share wire structs.
struct SpeakerEdgeEvent {
    uint32_t timelineCycles = 0;
    bool levelHigh = false; // speaker state AFTER this edge
};

enum class AudioPeerState : uint8_t {
    AudioOff = 0,
    AudioPairing,
    AudioConnecting,
    AudioConnected,
    AudioStreaming,
    AudioLost,
    AudioError,
};

inline const char *audioPeerStateName(AudioPeerState s) {
    switch (s) {
    case AudioPeerState::AudioOff:
        return "AUDIO_OFF";
    case AudioPeerState::AudioPairing:
        return "AUDIO_PAIRING";
    case AudioPeerState::AudioConnecting:
        return "AUDIO_CONNECTING";
    case AudioPeerState::AudioConnected:
        return "AUDIO_CONNECTED";
    case AudioPeerState::AudioStreaming:
        return "AUDIO_STREAMING";
    case AudioPeerState::AudioLost:
        return "AUDIO_LOST";
    case AudioPeerState::AudioError:
        return "AUDIO_ERROR";
    default:
        return "AUDIO_UNKNOWN";
    }
}

} // namespace audio
} // namespace blueshift
