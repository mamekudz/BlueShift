#pragma once

#include <cstddef>
#include <cstdint>

#include "protocol/extension_protocol.h"

namespace blueshift {
namespace protocol {

// Encode/decode EdgeBatch frames — no BLE / no A2DP dependencies.
class ExtensionCodec {
public:
    // Returns bytes written, or 0 on failure.
    static std::size_t encodeEdgeBatch(uint8_t *out, std::size_t outCap, uint16_t sequence,
                                       uint32_t baseCycle, const uint16_t *deltas,
                                       uint16_t edgeCount);

    // Returns true and fills outs; deltasOut must hold at least edgeCount.
    static bool decodeEdgeBatch(const uint8_t *in, std::size_t inLen, EdgeBatchHeader &hdr,
                                uint16_t *deltasOut, uint16_t deltasCap);

    static std::size_t encodeCaps(uint8_t *out, std::size_t outCap, const CapsPayload &caps);
    static bool decodeCaps(const uint8_t *in, std::size_t inLen, CapsPayload &caps);

    static std::size_t encodeControl(uint8_t *out, std::size_t outCap, StreamCommand cmd);
    static bool decodeControl(const uint8_t *in, std::size_t inLen, ControlPayload &ctrl);

    static std::size_t encodeSync(uint8_t *out, std::size_t outCap, const SyncMarkerPayload &sync);
    static bool decodeSync(const uint8_t *in, std::size_t inLen, SyncMarkerPayload &sync);
};

} // namespace protocol
} // namespace blueshift
