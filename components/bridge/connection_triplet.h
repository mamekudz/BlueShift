#pragma once

#include <cstdint>

namespace blueshift {

// Independent connection identities — do not overload "paired device".
enum class LinkRole : uint8_t {
    ClassicInput = 0, // HID device → BlueShift
    BleHost = 1,      // BlueShift → ESP][ / modern host
    A2dpSink = 2      // BlueShift → headphones/speaker
};

struct ConnectionTriplet {
    bool inputConnected = false;
    bool hostConnected = false;
    bool audioConnected = false;
};

// Audio is OPTIONAL — failure must not break HID bridging.
inline bool hidBridgeViable(const ConnectionTriplet &t) {
    return t.inputConnected && t.hostConnected;
}

inline bool audioPathViable(const ConnectionTriplet &t) {
    return t.hostConnected && t.audioConnected;
}

} // namespace blueshift
