#pragma once

#include <cstdint>

namespace blueshift {
namespace protocol {

// Transport-neutral BlueShift Extension Protocol v1.
// Not BLE GATT, not A2DP, not ESP][ emulator code.
// Status: SPECIFIED / HOST-TESTABLE — PHYSICALLY UNVERIFIED.
// UUIDs are STABLE (Protocol V1 freeze) — do not change casually.
// Single source of truth for GATT registration — do not duplicate elsewhere.

inline constexpr uint16_t kProtocolVersion = 1;

// ---------------------------------------------------------------------------
// Vendor 128-bit UUIDs (Bluetooth SIG 16-bit range NEVER used).
// Human-readable: 6b6c7565-5368-6966-74NN-000000000000
//   "blue" "Sh" "if" "t" + characteristic number NN
// Wire order below is little-endian as used by ESP-IDF esp_gatt_* APIs.
// ---------------------------------------------------------------------------

// Service: ...-7401-...
inline constexpr uint8_t kServiceUuid128[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x74, 0x66, 0x69, 0x68, 0x53, 0x65, 0x75, 0x6c, 0x6b};
// Capabilities (read): ...-7402-...
inline constexpr uint8_t kCapsUuid128[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x74, 0x66, 0x69, 0x68, 0x53, 0x65, 0x75, 0x6c, 0x6b};
// Control (write): ...-7403-...
inline constexpr uint8_t kControlUuid128[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x74, 0x66, 0x69, 0x68, 0x53, 0x65, 0x75, 0x6c, 0x6b};
// Edge stream (write from host): ...-7404-...
inline constexpr uint8_t kEdgeStreamUuid128[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x74, 0x66, 0x69, 0x68, 0x53, 0x65, 0x75, 0x6c, 0x6b};

inline constexpr const char *kServiceUuidString = "6b6c7565-5368-6966-7401-000000000000";
inline constexpr const char *kCapsUuidString = "6b6c7565-5368-6966-7402-000000000000";
inline constexpr const char *kControlUuidString = "6b6c7565-5368-6966-7403-000000000000";
inline constexpr const char *kEdgeStreamUuidString = "6b6c7565-5368-6966-7404-000000000000";

enum class Capability : uint32_t {
    None = 0,
    SpeakerEdgeStream = 1u << 0,
    // Architecture reserved — do NOT advertise until implemented:
    // Haptics = 1u << 1,
    // Status = 1u << 2,
};

enum class StreamCommand : uint8_t {
    Nop = 0,
    Start = 1,
    Stop = 2,
    Sync = 3,
    Reset = 4,
};

// Aliases for clarity with ESP][ docs
inline constexpr StreamCommand kCmdStartAudio = StreamCommand::Start;
inline constexpr StreamCommand kCmdStopAudio = StreamCommand::Stop;
inline constexpr StreamCommand kCmdSyncAudio = StreamCommand::Sync;
inline constexpr StreamCommand kCmdResetAudio = StreamCommand::Reset;

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
    DuplicateSequence = 6,
    HugeDelta = 7,
};

// V1 wire: batched uint16 deltas with escape for large gaps (C + escape).
enum class EdgeEncoding : uint8_t {
    FixedU16DeltaWithEscape = 0,
};

// Escape: delta == 0xFFFF means next 4 bytes are uint32 absolute cycle delta.
inline constexpr uint16_t kDeltaEscapeLarge = 0xFFFF;
inline constexpr uint32_t kMaxReasonableDeltaCycles = 1020484u * 5u; // ~5s @ Apple II clock

struct CapsPayload {
    uint16_t protocolVersion = kProtocolVersion;
    uint32_t capabilities = static_cast<uint32_t>(Capability::SpeakerEdgeStream);
    EdgeEncoding preferredEncoding = EdgeEncoding::FixedU16DeltaWithEscape;
    uint16_t maxEdgesPerFrame = 64;
};

struct ControlPayload {
    StreamCommand command = StreamCommand::Nop;
    uint8_t reserved = 0;
};

struct EdgeBatchHeader {
    uint8_t type = static_cast<uint8_t>(FrameType::EdgeBatch);
    uint8_t encoding = static_cast<uint8_t>(EdgeEncoding::FixedU16DeltaWithEscape);
    uint16_t sequence = 0;
    uint32_t baseCycle = 0;
    uint16_t edgeCount = 0;
};

inline constexpr uint16_t kMaxEdgesPerFrame = 64;
inline constexpr uint16_t kEdgeBatchHeaderBytes = 10;

struct SyncMarkerPayload {
    uint16_t sequence = 0;
    uint32_t timelineCycle = 0;
    uint8_t speakerLevel = 0; // 0=low, 1=high after sync
    uint8_t reason = 0;       // 0=start, 1=resync, 2=host_request
};

inline bool hasCapability(uint32_t mask, Capability bit) {
    return (mask & static_cast<uint32_t>(bit)) != 0;
}

} // namespace protocol
} // namespace blueshift
