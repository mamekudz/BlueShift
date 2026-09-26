#include "protocol/extension_codec.h"

#include <cstring>

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
                                            uint32_t baseCycle, const uint16_t *deltas,
                                            uint16_t edgeCount) {
    if (out == nullptr || deltas == nullptr || edgeCount == 0 || edgeCount > kMaxEdgesPerFrame) {
        return 0;
    }
    const std::size_t need = kEdgeBatchHeaderBytes + static_cast<std::size_t>(edgeCount) * 2u;
    if (outCap < need) {
        return 0;
    }
    out[0] = static_cast<uint8_t>(FrameType::EdgeBatch);
    out[1] = static_cast<uint8_t>(EdgeEncoding::FixedU16Delta);
    writeU16(out + 2, sequence);
    writeU32(out + 4, baseCycle);
    writeU16(out + 8, edgeCount);
    for (uint16_t i = 0; i < edgeCount; ++i) {
        writeU16(out + kEdgeBatchHeaderBytes + i * 2u, deltas[i]);
    }
    return need;
}

bool ExtensionCodec::decodeEdgeBatch(const uint8_t *in, std::size_t inLen, EdgeBatchHeader &hdr,
                                     uint16_t *deltasOut, uint16_t deltasCap) {
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
    if (hdr.encoding != static_cast<uint8_t>(EdgeEncoding::FixedU16Delta)) {
        return false;
    }
    if (hdr.edgeCount == 0 || hdr.edgeCount > kMaxEdgesPerFrame || hdr.edgeCount > deltasCap) {
        return false;
    }
    const std::size_t need = kEdgeBatchHeaderBytes + static_cast<std::size_t>(hdr.edgeCount) * 2u;
    if (inLen < need) {
        return false;
    }
    for (uint16_t i = 0; i < hdr.edgeCount; ++i) {
        deltasOut[i] = readU16(in + kEdgeBatchHeaderBytes + i * 2u);
    }
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
    if (out == nullptr || outCap < 8) {
        return 0;
    }
    out[0] = static_cast<uint8_t>(FrameType::SyncMarker);
    writeU16(out + 1, sync.sequence);
    writeU32(out + 3, sync.timelineCycle);
    out[7] = sync.reason;
    return 8;
}

bool ExtensionCodec::decodeSync(const uint8_t *in, std::size_t inLen, SyncMarkerPayload &sync) {
    if (in == nullptr || inLen < 8 || in[0] != static_cast<uint8_t>(FrameType::SyncMarker)) {
        return false;
    }
    sync.sequence = readU16(in + 1);
    sync.timelineCycle = readU32(in + 3);
    sync.reason = in[7];
    return true;
}

} // namespace protocol
} // namespace blueshift
