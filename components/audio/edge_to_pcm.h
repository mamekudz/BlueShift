#pragma once

#include <cstddef>
#include <cstdint>

namespace blueshift {
namespace audio {

// NTSC Apple II CPU cycle rate (common reference). DOCUMENTED community value.
inline constexpr uint32_t kApple2CyclesPerSecond = 1020484;

inline constexpr int16_t kPcmAmplitude = 16000;

struct SpeakerEdge {
    uint32_t absCycle = 0; // absolute Apple II cycle (wraps)
};

// Integrates speaker state across each PCM sample (sub-sample pulses matter).
 // Timeline is Apple II cycles — NOT BLE arrival time.
class EdgeToPcmRenderer {
public:
    explicit EdgeToPcmRenderer(uint32_t sampleRateHz = 44100);

    void reset(uint32_t startCycle = 0, bool speakerHigh = false);
    void setSampleRate(uint32_t sampleRateHz);

    uint32_t sampleRate() const {
        return sampleRateHz_;
    }

    // Push next edge (toggle). Edges must be non-decreasing in absCycle (wrap-aware soft).
    void pushEdge(uint32_t absCycle);

    // Render up to maxSamples using current edge queue + held level.
    // Returns samples written. Underrun fills silence and increments underruns_.
    std::size_t render(int16_t *out, std::size_t maxSamples);

    // Force-advance timeline by consuming held level (no new edges) — for tests.
    std::size_t renderSilenceHold(int16_t *out, std::size_t maxSamples);

    uint32_t underrunCount() const {
        return underruns_;
    }
    uint32_t overflowCount() const {
        return overflows_;
    }
    uint32_t samplesRendered() const {
        return samplesRendered_;
    }
    uint32_t timelineCycle() const {
        return timelineCycle_;
    }
    bool speakerHigh() const {
        return speakerHigh_;
    }

    // Fixed-point cycles per sample: (apple2Hz << 16) / sampleRate
    uint64_t cyclesPerSampleFp() const {
        return cyclesPerSampleFp_;
    }

private:
    static constexpr std::size_t kEdgeCap = 512;

    void recomputeRate();
    int16_t levelSample() const;
    std::size_t renderOne(int16_t *out);

    uint32_t sampleRateHz_ = 44100;
    uint64_t cyclesPerSampleFp_ = 0;
    uint64_t cycleFracFp_ = 0; // fractional accumulator
    uint32_t timelineCycle_ = 0;
    bool speakerHigh_ = false;
    SpeakerEdge edges_[kEdgeCap]{};
    std::size_t edgeHead_ = 0;
    std::size_t edgeSize_ = 0;
    uint32_t underruns_ = 0;
    uint32_t overflows_ = 0;
    uint32_t samplesRendered_ = 0;
};

} // namespace audio
} // namespace blueshift
