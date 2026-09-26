#pragma once

#include <cstddef>
#include <cstdint>

#include "protocol/extension_protocol.h"

namespace blueshift {
namespace protocol {

// Encode/decode V1 frames — no BLE / no A2DP dependencies.
class ExtensionCodec {
public:
    // deltas[i] is cycle gap before edge i (from previous edge or baseCycle).
    // Gaps >= 0xFFFF use escape encoding automatically.
    static std::size_t encodeEdgeBatch(uint8_t *out, std::size_t outCap, uint16_t sequence,
                                       uint32_t baseCycle, const uint32_t *deltas,
                                       uint16_t edgeCount);

    static bool decodeEdgeBatch(const uint8_t *in, std::size_t inLen, EdgeBatchHeader &hdr,
                                uint32_t *deltasOut, uint16_t deltasCap, uint16_t &decodedCount);

    static std::size_t encodeCaps(uint8_t *out, std::size_t outCap, const CapsPayload &caps);
    static bool decodeCaps(const uint8_t *in, std::size_t inLen, CapsPayload &caps);

    static std::size_t encodeControl(uint8_t *out, std::size_t outCap, StreamCommand cmd);
    static bool decodeControl(const uint8_t *in, std::size_t inLen, ControlPayload &ctrl);

    static std::size_t encodeSync(uint8_t *out, std::size_t outCap, const SyncMarkerPayload &sync);
    static bool decodeSync(const uint8_t *in, std::size_t inLen, SyncMarkerPayload &sync);
};

} // namespace protocol
} // namespace blueshift
