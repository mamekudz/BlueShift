#include "protocol/extension_codec.h"

namespace blueshift {
namespace protocol {
namespace {

void writeU16(uint8_t *p, uint16_t v) {
    p[0] = static_cast<uint8_t>(v & 0xFFu);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFFu);
}

void writeU32(uint8_t *p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v & 0xFFu);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFFu);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xFFu);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xFFu);
}

uint16_t readU16(const uint8_t *p) {
    return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
}

uint32_t readU32(const uint8_t *p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

} // namespace

std::size_t ExtensionCodec::encodeEdgeBatch(uint8_t *out, std::size_t outCap, uint16_t sequence,
                                            uint32_t baseCycle, const uint32_t *deltas,
                                            uint16_t edgeCount) {
    if (out == nullptr || deltas == nullptr || edgeCount == 0 || edgeCount > kMaxEdgesPerFrame) {
        return 0;
    }
    // Worst case: every delta escapes => 10 + edgeCount * 6
    const std::size_t worst = kEdgeBatchHeaderBytes + static_cast<std::size_t>(edgeCount) * 6u;
    if (outCap < worst && outCap < kEdgeBatchHeaderBytes + 2u) {
        return 0;
    }
    out[0] = static_cast<uint8_t>(FrameType::EdgeBatch);
    out[1] = static_cast<uint8_t>(EdgeEncoding::FixedU16DeltaWithEscape);
    writeU16(out + 2, sequence);
    writeU32(out + 4, baseCycle);
    writeU16(out + 8, edgeCount);

    std::size_t pos = kEdgeBatchHeaderBytes;
    for (uint16_t i = 0; i < edgeCount; ++i) {
        const uint32_t d = deltas[i];
        if (d == 0 || d > kMaxReasonableDeltaCycles) {
            return 0;
        }
        if (d < kDeltaEscapeLarge) {
            if (pos + 2 > outCap) {
                return 0;
            }
            writeU16(out + pos, static_cast<uint16_t>(d));
            pos += 2;
        } else {
            if (pos + 6 > outCap) {
                return 0;
            }
            writeU16(out + pos, kDeltaEscapeLarge);
            writeU32(out + pos + 2, d);
            pos += 6;
        }
    }
    return pos;
}

bool ExtensionCodec::decodeEdgeBatch(const uint8_t *in, std::size_t inLen, EdgeBatchHeader &hdr,
                                     uint32_t *deltasOut, uint16_t deltasCap,
                                     uint16_t &decodedCount) {
    decodedCount = 0;
    if (in == nullptr || deltasOut == nullptr || inLen < kEdgeBatchHeaderBytes) {
        return false;
    }
    if (in[0] != static_cast<uint8_t>(FrameType::EdgeBatch)) {
        return false;
    }
    hdr.type = in[0];
    hdr.encoding = in[1];
    hdr.sequence = readU16(in + 2);
    hdr.baseCycle = readU32(in + 4);
    hdr.edgeCount = readU16(in + 8);
    if (hdr.encoding != static_cast<uint8_t>(EdgeEncoding::FixedU16DeltaWithEscape)) {
        return false;
    }
    if (hdr.edgeCount == 0 || hdr.edgeCount > kMaxEdgesPerFrame || hdr.edgeCount > deltasCap) {
        return false;
    }
    std::size_t pos = kEdgeBatchHeaderBytes;
    for (uint16_t i = 0; i < hdr.edgeCount; ++i) {
        if (pos + 2 > inLen) {
            return false;
        }
        const uint16_t raw = readU16(in + pos);
        pos += 2;
        uint32_t d = 0;
        if (raw == kDeltaEscapeLarge) {
            if (pos + 4 > inLen) {
                return false;
            }
            d = readU32(in + pos);
            pos += 4;
        } else {
            d = raw;
        }
        if (d == 0 || d > kMaxReasonableDeltaCycles) {
            return false;
        }
        deltasOut[i] = d;
    }
    if (pos != inLen) {
        // Trailing bytes = malformed (strict).
        return false;
    }
    decodedCount = hdr.edgeCount;
    return true;
}

std::size_t ExtensionCodec::encodeCaps(uint8_t *out, std::size_t outCap, const CapsPayload &caps) {
    if (out == nullptr || outCap < 10) {
        return 0;
    }
    out[0] = static_cast<uint8_t>(FrameType::Caps);
    writeU16(out + 1, caps.protocolVersion);
    writeU32(out + 3, caps.capabilities);
    out[7] = static_cast<uint8_t>(caps.preferredEncoding);
    writeU16(out + 8, caps.maxEdgesPerFrame);
    return 10;
}

bool ExtensionCodec::decodeCaps(const uint8_t *in, std::size_t inLen, CapsPayload &caps) {
    if (in == nullptr || inLen < 10 || in[0] != static_cast<uint8_t>(FrameType::Caps)) {
        return false;
    }
    caps.protocolVersion = readU16(in + 1);
    caps.capabilities = readU32(in + 3);
    caps.preferredEncoding = static_cast<EdgeEncoding>(in[7]);
    caps.maxEdgesPerFrame = readU16(in + 8);
    return caps.protocolVersion == kProtocolVersion;
}

std::size_t ExtensionCodec::encodeControl(uint8_t *out, std::size_t outCap, StreamCommand cmd) {
    if (out == nullptr || outCap < 3) {
        return 0;
    }
    out[0] = static_cast<uint8_t>(FrameType::Control);
    out[1] = static_cast<uint8_t>(cmd);
    out[2] = 0;
    return 3;
}

bool ExtensionCodec::decodeControl(const uint8_t *in, std::size_t inLen, ControlPayload &ctrl) {
    if (in == nullptr || inLen < 3 || in[0] != static_cast<uint8_t>(FrameType::Control)) {
        return false;
    }
    ctrl.command = static_cast<StreamCommand>(in[1]);
    ctrl.reserved = in[2];
    return true;
}

std::size_t ExtensionCodec::encodeSync(uint8_t *out, std::size_t outCap,
                                       const SyncMarkerPayload &sync) {
    if (out == nullptr || outCap < 9) {
        return 0;
    }
    out[0] = static_cast<uint8_t>(FrameType::SyncMarker);
    writeU16(out + 1, sync.sequence);
    writeU32(out + 3, sync.timelineCycle);
    out[7] = sync.speakerLevel ? 1 : 0;
    out[8] = sync.reason;
    return 9;
}

bool ExtensionCodec::decodeSync(const uint8_t *in, std::size_t inLen, SyncMarkerPayload &sync) {
    if (in == nullptr || inLen < 9 || in[0] != static_cast<uint8_t>(FrameType::SyncMarker)) {
        return false;
    }
    sync.sequence = readU16(in + 1);
    sync.timelineCycle = readU32(in + 3);
    sync.speakerLevel = in[7] ? 1 : 0;
    sync.reason = in[8];
    return true;
}

} // namespace protocol
} // namespace blueshift
