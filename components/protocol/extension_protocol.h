#pragma once

#include <cstdint>

namespace blueshift {
namespace protocol {

// Transport-neutral BlueShift Extension Protocol v1.
 // Not BLE GATT, not A2DP, not ESP][ emulator code.
 // Status: DESIGNED / HOST-TESTABLE — PHYSICALLY UNVERIFIED.

inline constexpr uint16_t kProtocolVersion = 1;

// Vendor 128-bit UUID base (Bluetooth base UUID style, BlueShift-specific).
 // Base: 6b6c7565-5368-6966-7400-000000000000 ("blueShift" mnemonic packing).
 // Characteristic offsets: +0x0001.. — do not collide with Bluetooth SIG 16-bit UUIDs.
inline constexpr uint8_t kServiceUuid128[16] = {
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x74, 0x66, 0x69, 0x68, 0x53, 0x65, 0x75, 0x6c, 0x6b, 0x6b};
 // Note: stored little-endian wire order for GATT registration helpers.

enum class Capability : uint32_t {
    None = 0,
    SpeakerEdgeStream = 1u << 0,
    // Future bits (do NOT advertise until implemented):
    // Haptics = 1u << 1,
    // Status = 1u << 2,
};

enum class StreamCommand : uint8_t {
    Nop = 0,
    StartAudio = 1,
    StopAudio = 2,
    SyncAudio = 3,
    ResetAudio = 4,
};

enum class FrameType : uint8_t {
    Caps = 0x01,
    Control = 0x02,
    EdgeBatch = 0x10,
    SyncMarker = 0x11,
    Error = 0x7F,
};

enum class ProtocolError : uint8_t {
    None = 0,
    UnsupportedVersion = 1,
    Malformed = 2,
    Overflow = 3,
    SequenceGap = 4,
    Underrun = 5,
};

// Candidate packet formats (research §8) — V1 wire uses batched fixed uint16 (C).
enum class EdgeEncoding : uint8_t {
    FixedU16Delta = 0,      // A / C
    VarintDelta = 1,        // B — researched, not default
    TimestampBatch = 2,     // D — researched, not default
};

struct CapsPayload {
    uint16_t protocolVersion = kProtocolVersion;
    uint32_t capabilities = static_cast<uint32_t>(Capability::SpeakerEdgeStream);
    EdgeEncoding preferredEncoding = EdgeEncoding::FixedU16Delta;
    uint16_t maxEdgesPerFrame = 64;
};

struct ControlPayload {
    StreamCommand command = StreamCommand::Nop;
    uint8_t reserved = 0;
};

// Edge batch: Apple II speaker toggles; state bit optional because access toggles.
struct EdgeBatchHeader {
    uint8_t type = static_cast<uint8_t>(FrameType::EdgeBatch);
    uint8_t encoding = static_cast<uint8_t>(EdgeEncoding::FixedU16Delta);
    uint16_t sequence = 0;
    uint32_t baseCycle = 0; // absolute cycle at first edge (wraps)
    uint16_t edgeCount = 0;
};

inline constexpr uint16_t kMaxEdgesPerFrame = 64;
inline constexpr uint16_t kEdgeBatchHeaderBytes = 10;
 // type(1)+encoding(1)+seq(2)+baseCycle(4)+edgeCount(2)

struct SyncMarkerPayload {
    uint16_t sequence = 0;
    uint32_t timelineCycle = 0;
    uint8_t reason = 0; // 0=start, 1=resync, 2=host_request
};

inline bool hasCapability(uint32_t mask, Capability bit) {
    return (mask & static_cast<uint32_t>(bit)) != 0;
}

} // namespace protocol
} // namespace blueshift
