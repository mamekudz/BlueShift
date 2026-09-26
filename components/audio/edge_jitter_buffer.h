#pragma once

#include <cstddef>
#include <cstdint>

#include "audio/edge_to_pcm.h"
#include "bridge/bounded_queue.h"

namespace blueshift {
namespace audio {

struct TimedEdge {
    uint32_t absCycle = 0;
};

// BLE receive path → bounded timing buffer (no SBC/PCM in callback).
// Apple II timeline from deltas — NOT BLE arrival time.
class EdgeJitterBuffer {
public:
    static constexpr std::size_t kCapacity = 1024;
    // Provisional development target (~few ms of edges at game rates). Configurable later.
    static constexpr std::size_t kDefaultTargetDepth = 128;
    static constexpr uint32_t kDefaultTargetLatencyMs = 40;

    void reset() {
        q_.clear();
        expectedSeq_ = 0;
        haveSeq_ = false;
        sequenceGaps_ = 0;
        underruns_ = 0;
        resyncs_ = 0;
        targetDepth_ = kDefaultTargetDepth;
        targetLatencyMs_ = kDefaultTargetLatencyMs;
    }

    void setTargetDepth(std::size_t depth) {
        targetDepth_ = depth == 0 ? 1 : depth;
    }

    void setTargetLatencyMs(uint32_t ms) {
        targetLatencyMs_ = ms;
    }

    std::size_t targetDepth() const {
        return targetDepth_;
    }

    uint32_t targetLatencyMs() const {
        return targetLatencyMs_;
    }

    bool readyToStream() const {
        return q_.size() >= targetDepth_;
    }

    bool push(uint32_t absCycle) {
        return q_.push(TimedEdge{absCycle}, QueueOverflowPolicy::DropOldest);
    }

    // deltas are uint32 cycle gaps (escape-decoded already).
    bool pushBatch(uint16_t sequence, uint32_t baseCycle, const uint32_t *deltas, uint16_t count) {
        if (deltas == nullptr && count > 0) {
            return false;
        }
        if (haveSeq_) {
            const uint16_t expect = static_cast<uint16_t>(expectedSeq_ + 1u);
            if (sequence != expect) {
                ++sequenceGaps_;
                ++resyncs_;
            }
        }
        expectedSeq_ = sequence;
        haveSeq_ = true;
        uint32_t cycle = baseCycle;
        bool ok = true;
        for (uint16_t i = 0; i < count; ++i) {
            cycle += deltas[i];
            if (!push(cycle)) {
                ok = false;
            }
        }
        return ok;
    }

    void applySync(uint16_t sequence, uint32_t timelineCycle) {
        expectedSeq_ = sequence;
        haveSeq_ = true;
        ++resyncs_;
        (void)timelineCycle;
    }

    bool pop(TimedEdge &out) {
        if (!q_.pop(out)) {
            ++underruns_;
            return false;
        }
        return true;
    }

    // Drain without counting underrun (worker moving edges to renderer).
    bool tryPop(TimedEdge &out) {
        return q_.pop(out);
    }

    std::size_t size() const {
        return q_.size();
    }
    uint32_t overflowCount() const {
        return q_.overflowCount();
    }
    uint32_t sequenceGaps() const {
        return sequenceGaps_;
    }
    uint32_t underrunCount() const {
        return underruns_;
    }
    uint32_t resyncCount() const {
        return resyncs_;
    }

private:
    BoundedQueue<TimedEdge, kCapacity> q_{};
    uint16_t expectedSeq_ = 0;
    bool haveSeq_ = false;
    uint32_t sequenceGaps_ = 0;
    uint32_t underruns_ = 0;
    uint32_t resyncs_ = 0;
    std::size_t targetDepth_ = kDefaultTargetDepth;
    uint32_t targetLatencyMs_ = kDefaultTargetLatencyMs;
};

} // namespace audio
} // namespace blueshift
