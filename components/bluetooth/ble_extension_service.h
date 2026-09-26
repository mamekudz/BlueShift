#pragma once

#include <cstddef>
#include <cstdint>

#include "protocol/extension_codec.h"
#include "protocol/extension_protocol.h"

namespace blueshift {
namespace bluetooth {

// GATT server façade for BlueShift Extension Service (alongside BLE HID).
// Status: INTERFACE / HOST-TESTABLE framing — PHYSICALLY UNVERIFIED.
// Standard HID hosts ignore this service.
class BleExtensionService {
public:
    bool begin() {
        enabled_ = true;
        return true;
    }
    void end() {
        enabled_ = false;
        streamActive_ = false;
    }

    bool enabled() const {
        return enabled_;
    }
    bool streamActive() const {
        return streamActive_;
    }

    // Caps characteristic payload (read).
    std::size_t readCaps(uint8_t *out, std::size_t cap) const {
        protocol::CapsPayload caps{};
        caps.capabilities = static_cast<uint32_t>(protocol::Capability::SpeakerEdgeStream);
        return protocol::ExtensionCodec::encodeCaps(out, cap, caps);
    }

    // Control characteristic write from ESP][ host.
    bool onControlWrite(const uint8_t *data, std::size_t len) {
        protocol::ControlPayload ctrl{};
        if (!protocol::ExtensionCodec::decodeControl(data, len, ctrl)) {
            return false;
        }
        switch (ctrl.command) {
        case protocol::StreamCommand::Start:
            streamActive_ = true;
            break;
        case protocol::StreamCommand::Stop:
        case protocol::StreamCommand::Reset:
            streamActive_ = false;
            break;
        default:
            break;
        }
        return true;
    }

    // Edge stream characteristic — enqueue only; caller owns AudioPipeline.
    using EdgeSink = void (*)(const uint8_t *data, std::size_t len, void *ctx);
    void setEdgeSink(EdgeSink sink, void *ctx) {
        sink_ = sink;
        sinkCtx_ = ctx;
    }

    bool onEdgeWrite(const uint8_t *data, std::size_t len) {
        if (!streamActive_ || sink_ == nullptr) {
            return false;
        }
        sink_(data, len, sinkCtx_);
        return true;
    }

private:
    bool enabled_ = false;
    bool streamActive_ = false;
    EdgeSink sink_ = nullptr;
    void *sinkCtx_ = nullptr;
};

} // namespace bluetooth
} // namespace blueshift
