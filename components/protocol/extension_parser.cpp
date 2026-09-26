#include "protocol/extension_parser.h"

namespace blueshift {
namespace protocol {

ParseResult ExtensionParser::noteSequence(uint16_t seq, bool allowDuplicate) {
    if (!haveSeq_) {
        expectedSeq_ = seq;
        haveSeq_ = true;
        return ParseResult::Ok;
    }
    if (seq == expectedSeq_) {
        if (allowDuplicate) {
            ++stats_.duplicates;
            return ParseResult::DuplicateSequence;
        }
        ++stats_.duplicates;
        return ParseResult::DuplicateSequence;
    }
    const uint16_t next = static_cast<uint16_t>(expectedSeq_ + 1u);
    if (seq != next) {
        ++stats_.sequenceErrors;
        expectedSeq_ = seq;
        return ParseResult::SequenceGap;
    }
    expectedSeq_ = seq;
    return ParseResult::Ok;
}

ParseResult ExtensionParser::handleControl(const uint8_t *data, std::size_t len) {
    ControlPayload ctrl{};
    if (!ExtensionCodec::decodeControl(data, len, ctrl)) {
        ++stats_.malformed;
        return ParseResult::Malformed;
    }
    switch (ctrl.command) {
    case StreamCommand::Start:
        accepting_ = true;
        break;
    case StreamCommand::Stop:
        accepting_ = false;
        break;
    case StreamCommand::Reset:
        reset();
        accepting_ = false;
        break;
    case StreamCommand::Sync:
        // Prefer SyncMarker frame; control SYNC alone is soft hint.
        ++stats_.resyncs;
        break;
    case StreamCommand::Nop:
    default:
        break;
    }
    return ParseResult::Ok;
}

ParseResult ExtensionParser::handleSync(const uint8_t *data, std::size_t len) {
    SyncMarkerPayload sync{};
    if (!ExtensionCodec::decodeSync(data, len, sync)) {
        ++stats_.malformed;
        return ParseResult::Malformed;
    }
    const ParseResult seq = noteSequence(sync.sequence, true);
    if (seq == ParseResult::DuplicateSequence) {
        return seq;
    }
    timeline_ = sync.timelineCycle;
    levelHigh_ = sync.speakerLevel != 0;
    accepting_ = true;
    ++stats_.resyncs;
    expectedSeq_ = sync.sequence;
    haveSeq_ = true;
    return seq == ParseResult::SequenceGap ? ParseResult::SequenceGap : ParseResult::Ok;
}

ParseResult ExtensionParser::handleEdgeBatch(const uint8_t *data, std::size_t len,
                                             ParsedSpeakerEdge *out, uint16_t outCap,
                                             uint16_t &emitted) {
    emitted = 0;
    if (!accepting_) {
        // Ignore edges until START/SYNC — not an error.
        return ParseResult::Ok;
    }
    EdgeBatchHeader hdr{};
    uint32_t deltas[kMaxEdgesPerFrame];
    uint16_t count = 0;
    if (!ExtensionCodec::decodeEdgeBatch(data, len, hdr, deltas, kMaxEdgesPerFrame, count)) {
        ++stats_.malformed;
        return ParseResult::Malformed;
    }
    const ParseResult seq = noteSequence(hdr.sequence, false);
    if (seq == ParseResult::DuplicateSequence) {
        return seq;
    }
    // On gap: still accept after resync flag (caller may soft-resync timeline from baseCycle).
    if (seq == ParseResult::SequenceGap) {
        timeline_ = hdr.baseCycle;
        ++stats_.resyncs;
    } else if (hdr.baseCycle != timeline_) {
        // Soft align to packet base if host restarted timeline without SYNC.
        timeline_ = hdr.baseCycle;
    }

    if (count > outCap) {
        ++stats_.malformed;
        return ParseResult::Overflow;
    }
    for (uint16_t i = 0; i < count; ++i) {
        timeline_ += deltas[i];
        levelHigh_ = !levelHigh_;
        out[i].timelineCycles = timeline_;
        out[i].levelHigh = levelHigh_;
        ++emitted;
        ++stats_.edges;
    }
    return seq;
}

ParseResult ExtensionParser::ingest(const uint8_t *data, std::size_t len, ParsedSpeakerEdge *out,
                                    uint16_t outCap, uint16_t &emitted) {
    emitted = 0;
    if (data == nullptr || len == 0) {
        ++stats_.malformed;
        return ParseResult::Malformed;
    }
    ++stats_.packets;
    const auto type = static_cast<FrameType>(data[0]);
    switch (type) {
    case FrameType::Caps: {
        CapsPayload caps{};
        if (!ExtensionCodec::decodeCaps(data, len, caps)) {
            ++stats_.malformed;
            return caps.protocolVersion != 0 && caps.protocolVersion != kProtocolVersion
                       ? ParseResult::UnsupportedVersion
                       : ParseResult::Malformed;
        }
        return ParseResult::Ok;
    }
    case FrameType::Control:
        return handleControl(data, len);
    case FrameType::SyncMarker:
        return handleSync(data, len);
    case FrameType::EdgeBatch:
        return handleEdgeBatch(data, len, out, outCap, emitted);
    default:
        ++stats_.malformed;
        return ParseResult::Malformed;
    }
}

} // namespace protocol
} // namespace blueshift
