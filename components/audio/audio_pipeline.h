#pragma once

#include <cstddef>
#include <cstdint>

#include "audio/a2dp_audio_output.h"
#include "audio/edge_jitter_buffer.h"
#include "audio/edge_to_pcm.h"
#include "audio/speaker_edge.h"
#include "protocol/extension_parser.h"

namespace blueshift {
namespace audio {

struct AudioPipelineDiagnostics {
    uint16_t protocolVersion = protocol::kProtocolVersion;
    uint32_t packets = 0;
    uint32_t edges = 0;
    uint32_t sequenceErrors = 0;
    uint32_t resyncs = 0;
    uint32_t underruns = 0;
    uint32_t overruns = 0;
    uint32_t pcmSamples = 0;
    uint32_t sampleRate = 44100;
    bool audioConnected = false;
    bool streaming = false;
    uint32_t edgeQueueDepth = 0;
    uint32_t jitterDepth = 0;
    uint32_t pcmQueueDepth = 0;
};

// Orchestrates: BLE frames → parser → jitter → PCM → A2dpAudioOutput.
// HID bridge is intentionally outside this class (failure isolation).
class AudioPipeline {
public:
    explicit AudioPipeline(A2dpAudioOutput &output) : output_(output) {}

    void reset() {
        parser_.reset();
        jitter_.reset();
        renderer_.reset(0, false);
        hidStillOk_ = true;
        streamEnabled_ = false;
        pendingResync_ = false;
        resyncCycle_ = 0;
        resyncLevel_ = false;
    }

    void setSampleRate(uint32_t hz) {
        renderer_.setSampleRate(hz);
        output_.setSampleRateHz(hz);
    }

    // BLE notify/write callback path: validate + enqueue only (no PCM/SBC).
    protocol::ParseResult onExtensionPayload(const uint8_t *data, std::size_t len) {
        protocol::ParsedSpeakerEdge edges[protocol::kMaxEdgesPerFrame];
        uint16_t emitted = 0;
        const auto r = parser_.ingest(data, len, edges, protocol::kMaxEdgesPerFrame, emitted);

        if (data != nullptr && len > 0 &&
            static_cast<protocol::FrameType>(data[0]) == protocol::FrameType::SyncMarker) {
            protocol::SyncMarkerPayload sync{};
            if (protocol::ExtensionCodec::decodeSync(data, len, sync)) {
                pendingResync_ = true;
                resyncCycle_ = sync.timelineCycle;
                resyncLevel_ = sync.speakerLevel != 0;
                jitter_.reset();
                jitter_.applySync(sync.sequence, sync.timelineCycle);
            }
        }

        if (r == protocol::ParseResult::Malformed ||
            r == protocol::ParseResult::UnsupportedVersion) {
            return r;
        }

        for (uint16_t i = 0; i < emitted; ++i) {
            (void)jitter_.push(edges[i].timelineCycles);
        }
        if (r == protocol::ParseResult::SequenceGap) {
            jitter_.applySync(parser_.expectedSequence(), 0);
        }
        return r;
    }

    void setStreamEnabled(bool on) {
        streamEnabled_ = on;
        if (!on) {
            output_.stopStream();
        }
    }

    bool streamEnabled() const {
        return streamEnabled_;
    }

    // Audio worker: drain jitter → renderer → supply PCM. Never call from BLE cb.
    std::size_t pumpPcm(std::size_t maxSamples) {
        if (!streamEnabled_) {
            return 0;
        }
        if (output_.state() != AudioPeerState::AudioStreaming &&
            output_.state() != AudioPeerState::AudioConnected) {
            return 0;
        }
        if (pendingResync_) {
            renderer_.reset(resyncCycle_, resyncLevel_);
            pendingResync_ = false;
        }
        // Move pending edges into renderer (bounded).
        TimedEdge te{};
        unsigned moved = 0;
        while (moved < 256 && jitter_.tryPop(te)) {
            renderer_.pushEdge(te.absCycle);
            ++moved;
        }
        if (output_.state() == AudioPeerState::AudioConnected) {
            output_.startStream();
        }
        int16_t tmp[256];
        std::size_t total = 0;
        while (total < maxSamples) {
            const std::size_t chunk = maxSamples - total;
            const std::size_t n = chunk > 256 ? 256 : chunk;
            // If no edges and not holding useful audio, silence-hold on underrun path
            // is caller policy; here we render held level (square continues).
            renderer_.render(tmp, n);
            output_.supplyPcm(tmp, n);
            total += n;
        }
        return total;
    }

    std::size_t onA2dpPull(int16_t *buf, std::size_t maxSamples) {
        return output_.pullPcm(buf, maxSamples);
    }

    bool hidBridgeUnaffected() const {
        return hidStillOk_;
    }

    void markHidOk(bool ok) {
        hidStillOk_ = ok;
    }

    void simulateAudioDisconnect() {
        output_.disconnect();
        streamEnabled_ = false;
    }

    AudioPipelineDiagnostics diagnostics() const {
        AudioPipelineDiagnostics d;
        d.protocolVersion = protocol::kProtocolVersion;
        d.packets = parser_.stats().packets;
        d.edges = parser_.stats().edges;
        d.sequenceErrors = parser_.stats().sequenceErrors + jitter_.sequenceGaps();
        d.resyncs = parser_.stats().resyncs + jitter_.resyncCount();
        d.underruns = renderer_.underrunCount() + output_.underrunCount() + jitter_.underrunCount();
        d.overruns = renderer_.overflowCount() + output_.overrunCount() + jitter_.overflowCount();
        d.pcmSamples = output_.pcmSamplesSupplied();
        d.sampleRate = output_.sampleRateHz();
        d.audioConnected = output_.state() == AudioPeerState::AudioConnected ||
                           output_.state() == AudioPeerState::AudioStreaming;
        d.streaming = output_.isStreaming();
        d.edgeQueueDepth = static_cast<uint32_t>(jitter_.size());
        d.jitterDepth = d.edgeQueueDepth;
        d.pcmQueueDepth = static_cast<uint32_t>(output_.pcmQueueDepth());
        return d;
    }

    protocol::ExtensionParser &parser() {
        return parser_;
    }
    EdgeJitterBuffer &jitter() {
        return jitter_;
    }
    EdgeToPcmRenderer &renderer() {
        return renderer_;
    }
    A2dpAudioOutput &output() {
        return output_;
    }

private:
    A2dpAudioOutput &output_;
    protocol::ExtensionParser parser_{};
    EdgeJitterBuffer jitter_{};
    EdgeToPcmRenderer renderer_{44100};
    bool streamEnabled_ = false;
    bool hidStillOk_ = true;
    bool pendingResync_ = false;
    uint32_t resyncCycle_ = 0;
    bool resyncLevel_ = false;
};

} // namespace audio
} // namespace blueshift
