#pragma once

#include <cstddef>
#include <cstdint>

#include "protocol/extension_codec.h"
#include "protocol/extension_protocol.h"

namespace blueshift {
namespace protocol {

struct ParsedSpeakerEdge {
    uint32_t timelineCycles = 0;
    bool levelHigh = false; // level AFTER this edge (toggle applied)
};

enum class ParseResult : uint8_t {
    Ok = 0,
    NeedMore,
    UnsupportedVersion,
    Malformed,
    SequenceGap,
    DuplicateSequence,
    HugeDelta,
    Overflow
};

struct ExtensionParserStats {
    uint32_t packets = 0;
    uint32_t edges = 0;
    uint32_t sequenceErrors = 0;
    uint32_t resyncs = 0;
    uint32_t malformed = 0;
    uint32_t duplicates = 0;
};

// Untrusted BLE payload → SpeakerEdge events. No A2DP / no OLED.
class ExtensionParser {
public:
    void reset() {
        haveSeq_ = false;
        expectedSeq_ = 0;
        timeline_ = 0;
        levelHigh_ = false;
        accepting_ = false;
        stats_ = {};
    }

    bool accepting() const {
        return accepting_;
    }

    const ExtensionParserStats &stats() const {
        return stats_;
    }

    uint16_t expectedSequence() const {
        return expectedSeq_;
    }

    // Push one GATT write/notification payload. Emits edges into out[] (cap).
    ParseResult ingest(const uint8_t *data, std::size_t len, ParsedSpeakerEdge *out,
                       uint16_t outCap, uint16_t &emitted);

private:
    ParseResult handleControl(const uint8_t *data, std::size_t len);
    ParseResult handleSync(const uint8_t *data, std::size_t len);
    ParseResult handleEdgeBatch(const uint8_t *data, std::size_t len, ParsedSpeakerEdge *out,
                                uint16_t outCap, uint16_t &emitted);
    ParseResult noteSequence(uint16_t seq, bool allowDuplicate);

    bool haveSeq_ = false;
    uint16_t expectedSeq_ = 0;
    uint32_t timeline_ = 0;
    bool levelHigh_ = false;
    bool accepting_ = false;
    ExtensionParserStats stats_{};
};

} // namespace protocol
} // namespace blueshift
