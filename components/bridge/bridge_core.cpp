#include "bridge/bridge_core.h"

namespace blueshift {

const char *bridgeStateName(BridgeState state) {
    switch (state) {
    case BridgeState::Boot:
        return "BOOT";
    case BridgeState::Idle:
        return "IDLE";
    case BridgeState::PairingInput:
        return "PAIRING_INPUT";
    case BridgeState::InputConnected:
        return "INPUT_CONNECTED";
    case BridgeState::PairingOutput:
        return "PAIRING_OUTPUT";
    case BridgeState::OutputConnected:
        return "OUTPUT_CONNECTED";
    case BridgeState::Bridging:
        return "BRIDGING";
    case BridgeState::InputLost:
        return "INPUT_LOST";
    case BridgeState::OutputLost:
        return "OUTPUT_LOST";
    case BridgeState::Error:
        return "ERROR";
    }
    return "UNKNOWN";
}

bool BridgeCore::apply(BridgeEvent event) {
    const BridgeState prev = state_;

    switch (event) {
    case BridgeEvent::BootDone:
        if (state_ == BridgeState::Boot) {
            state_ = BridgeState::Idle;
        }
        break;
    case BridgeEvent::StartPairInput:
        if (state_ == BridgeState::Idle || state_ == BridgeState::OutputConnected ||
            state_ == BridgeState::InputLost) {
            state_ = BridgeState::PairingInput;
        }
        break;
    case BridgeEvent::InputPaired:
        inputLinked_ = true;
        if (outputLinked_) {
            state_ = BridgeState::Bridging;
        } else if (state_ == BridgeState::PairingInput || state_ == BridgeState::Idle) {
            state_ = BridgeState::InputConnected;
        }
        break;
    case BridgeEvent::InputGone:
        inputLinked_ = false;
        ++releaseCount_;
        if (outputLinked_) {
            state_ = BridgeState::InputLost;
        } else {
            state_ = BridgeState::Idle;
        }
        break;
    case BridgeEvent::StartPairOutput:
        if (state_ == BridgeState::Idle || state_ == BridgeState::InputConnected ||
            state_ == BridgeState::OutputLost) {
            state_ = BridgeState::PairingOutput;
        }
        break;
    case BridgeEvent::OutputPaired:
        outputLinked_ = true;
        if (inputLinked_) {
            state_ = BridgeState::Bridging;
        } else {
            state_ = BridgeState::OutputConnected;
        }
        break;
    case BridgeEvent::OutputGone:
        outputLinked_ = false;
        ++releaseCount_;
        if (inputLinked_) {
            state_ = BridgeState::OutputLost;
        } else {
            state_ = BridgeState::Idle;
        }
        break;
    case BridgeEvent::EnterError:
        state_ = BridgeState::Error;
        break;
    case BridgeEvent::ClearError:
    case BridgeEvent::BackToIdle:
        if (!inputLinked_ && !outputLinked_) {
            state_ = BridgeState::Idle;
        } else if (inputLinked_ && outputLinked_) {
            state_ = BridgeState::Bridging;
        } else if (inputLinked_) {
            state_ = BridgeState::InputConnected;
        } else {
            state_ = BridgeState::OutputConnected;
        }
        break;
    }

    return state_ != prev;
}

bool BridgeCore::onNormalizedInput(const NormalizedInput &in, NormalizedInput &out) {
    lastKind_ = in.kind;
    if (state_ != BridgeState::Bridging) {
        return false;
    }
    out = in;
    return true;
}

NormalizedInput BridgeCore::makeReleaseSnapshot(NormalizedInput::Kind kind) const {
    NormalizedInput snap;
    snap.kind = kind;
    switch (kind) {
    case NormalizedInput::Kind::Keyboard:
        snap.keyboard.clear();
        break;
    case NormalizedInput::Kind::Mouse:
        snap.mouse.clearAll();
        break;
    case NormalizedInput::Kind::Gamepad:
        snap.gamepad.clear();
        break;
    case NormalizedInput::Kind::Consumer:
        snap.consumer.clear();
        break;
    case NormalizedInput::Kind::None:
        break;
    }
    return snap;
}

} // namespace blueshift
