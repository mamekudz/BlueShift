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
class EdgeJitterBuffer {
public:
    static constexpr std::size_t kCapacity = 1024;

    void reset() {
        q_.clear();
        expectedSeq_ = 0;
        haveSeq_ = false;
        sequenceGaps_ = 0;
        underruns_ = 0;
    }

    bool push(uint32_t absCycle) {
        return q_.push(TimedEdge{absCycle}, QueueOverflowPolicy::DropOldest);
    }

    bool pushBatch(uint16_t sequence, uint32_t baseCycle, const uint16_t *deltas, uint16_t count) {
        if (haveSeq_) {
            const uint16_t expect = static_cast<uint16_t>(expectedSeq_ + 1u);
            if (sequence != expect) {
                ++sequenceGaps_;
                // Accept but flag — caller may SYNC.
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

    bool pop(TimedEdge &out) {
        if (!q_.pop(out)) {
            ++underruns_;
            return false;
        }
        return true;
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

    // Target depth before starting A2DP pull — provisional latency budget.
    static constexpr std::size_t kTargetDepth = 128; // edges, not ms — tune later

private:
    BoundedQueue<TimedEdge, kCapacity> q_{};
    uint16_t expectedSeq_ = 0;
    bool haveSeq_ = false;
    uint32_t sequenceGaps_ = 0;
    uint32_t underruns_ = 0;
};

} // namespace audio
} // namespace blueshift
