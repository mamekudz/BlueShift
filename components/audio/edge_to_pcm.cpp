#include "audio/edge_to_pcm.h"

namespace blueshift {
namespace audio {

EdgeToPcmRenderer::EdgeToPcmRenderer(uint32_t sampleRateHz) {
    setSampleRate(sampleRateHz);
}

void EdgeToPcmRenderer::recomputeRate() {
    if (sampleRateHz_ == 0) {
        sampleRateHz_ = 44100;
    }
    cyclesPerSampleFp_ =
        (static_cast<uint64_t>(kApple2CyclesPerSecond) << 16) / sampleRateHz_;
}

void EdgeToPcmRenderer::setSampleRate(uint32_t sampleRateHz) {
    sampleRateHz_ = sampleRateHz;
    recomputeRate();
}

void EdgeToPcmRenderer::reset(uint32_t startCycle, bool speakerHigh) {
    timelineCycle_ = startCycle;
    speakerHigh_ = speakerHigh;
    edgeHead_ = 0;
    edgeSize_ = 0;
    cycleFracFp_ = 0;
    underruns_ = 0;
    overflows_ = 0;
    samplesRendered_ = 0;
}

int16_t EdgeToPcmRenderer::levelSample() const {
    return speakerHigh_ ? kPcmAmplitude : static_cast<int16_t>(-kPcmAmplitude);
}

void EdgeToPcmRenderer::pushEdge(uint32_t absCycle) {
    if (edgeSize_ >= kEdgeCap) {
        ++overflows_;
        // Drop oldest to keep newest timing (bounded).
        edgeHead_ = (edgeHead_ + 1) % kEdgeCap;
        --edgeSize_;
    }
    edges_[(edgeHead_ + edgeSize_) % kEdgeCap] = SpeakerEdge{absCycle};
    ++edgeSize_;
}

std::size_t EdgeToPcmRenderer::renderOne(int16_t *out) {
    cycleFracFp_ += cyclesPerSampleFp_;
    const uint32_t intervalCycles = static_cast<uint32_t>(cycleFracFp_ >> 16);
    cycleFracFp_ &= 0xFFFFu;
    if (intervalCycles == 0) {
        out[0] = 0;
        ++samplesRendered_;
        return 1;
    }

    uint32_t remaining = intervalCycles;
    int64_t integral = 0;

    while (remaining > 0) {
        uint32_t run = remaining;
        bool toggleAtEnd = false;

        if (edgeSize_ > 0) {
            const uint32_t nextAbs = edges_[edgeHead_].absCycle;
            const uint32_t delta = nextAbs - timelineCycle_;
            if (delta < remaining) {
                run = delta;
                toggleAtEnd = true;
            } else if (delta == 0) {
                // Edge exactly now: zero-width segment, toggle, continue.
                edgeHead_ = (edgeHead_ + 1) % kEdgeCap;
                --edgeSize_;
                speakerHigh_ = !speakerHigh_;
                continue;
            }
        }

        if (run > 0) {
            const int64_t lvl = speakerHigh_ ? 1 : -1;
            integral += lvl * static_cast<int64_t>(run);
            timelineCycle_ += run;
            remaining -= run;
        }

        if (toggleAtEnd) {
            edgeHead_ = (edgeHead_ + 1) % kEdgeCap;
            --edgeSize_;
            speakerHigh_ = !speakerHigh_;
        } else {
            // No edge inside remaining — consume rest at current level.
            break;
        }
    }

    if (remaining > 0) {
        const int64_t lvl = speakerHigh_ ? 1 : -1;
        integral += lvl * static_cast<int64_t>(remaining);
        timelineCycle_ += remaining;
    }

    const int64_t avg = integral / static_cast<int64_t>(intervalCycles);
    out[0] = static_cast<int16_t>(avg * kPcmAmplitude);
    ++samplesRendered_;
    return 1;
}

std::size_t EdgeToPcmRenderer::render(int16_t *out, std::size_t maxSamples) {
    if (out == nullptr || maxSamples == 0) {
        return 0;
    }
    std::size_t n = 0;
    while (n < maxSamples) {
        // Soft underrun: no edges and we still render held level (square continues).
        // True underrun for "data late" is tracked when caller requests silence fill.
        renderOne(out + n);
        ++n;
    }
    return n;
}

std::size_t EdgeToPcmRenderer::renderSilenceHold(int16_t *out, std::size_t maxSamples) {
    if (out == nullptr || maxSamples == 0) {
        return 0;
    }
    for (std::size_t i = 0; i < maxSamples; ++i) {
        out[i] = 0;
        // Advance timeline without edges → prevents replaying stale audio later.
        cycleFracFp_ += cyclesPerSampleFp_;
        const uint32_t intervalCycles = static_cast<uint32_t>(cycleFracFp_ >> 16);
        cycleFracFp_ &= 0xFFFFu;
        timelineCycle_ += intervalCycles;
        ++samplesRendered_;
        ++underruns_;
    }
    // Drop pending edges past the jumped timeline to resync.
    while (edgeSize_ > 0) {
        const uint32_t nextAbs = edges_[edgeHead_].absCycle;
        const uint32_t delta = nextAbs - timelineCycle_;
        // If edge is "behind" by large amount, drop; simple: drop all on underrun flush.
        (void)delta;
        edgeHead_ = (edgeHead_ + 1) % kEdgeCap;
        --edgeSize_;
    }
    return maxSamples;
}

} // namespace audio
} // namespace blueshift
