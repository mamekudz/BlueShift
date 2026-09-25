#pragma once

#include <cstdint>

#include "hid/normalized_hid.h"

namespace blueshift {

enum class BridgeState : uint8_t {
    Boot = 0,
    Idle,
    PairingInput,
    InputConnected,
    PairingOutput,
    OutputConnected,
    Bridging,
    InputLost,
    OutputLost,
    Error
};

enum class BridgeEvent : uint8_t {
    BootDone,
    StartPairInput,
    InputPaired,
    InputGone,
    StartPairOutput,
    OutputPaired,
    OutputGone,
    EnterError,
    ClearError,
    BackToIdle
};

const char *bridgeStateName(BridgeState state);

class BridgeCore {
public:
    BridgeState state() const {
        return state_;
    }

    bool inputLinked() const {
        return inputLinked_;
    }

    bool outputLinked() const {
        return outputLinked_;
    }

    bool apply(BridgeEvent event);

    // Feed newest normalized gamepad/mouse/keyboard. Returns whether an output
    // action should be emitted (only while Bridging).
    bool onNormalizedInput(const NormalizedInput &in, NormalizedInput &out);

    // On input/output loss: produce a cleared snapshot to release stuck controls.
    NormalizedInput makeReleaseSnapshot(NormalizedInput::Kind kind) const;

    std::uint32_t releaseCount() const {
        return releaseCount_;
    }

private:
    BridgeState state_ = BridgeState::Boot;
    bool inputLinked_ = false;
    bool outputLinked_ = false;
    NormalizedInput::Kind lastKind_ = NormalizedInput::Kind::None;
    std::uint32_t releaseCount_ = 0;
};

} // namespace blueshift
