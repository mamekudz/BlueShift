#pragma once

#include <cstdint>

namespace blueshift {

// Normalized HID models — transport-independent.
// Axes: int16_t in range [-32767, 32767], 0 = center.
// Triggers: uint8_t 0..255.
// Buttons: bitfield.

struct KeyboardState {
    uint8_t modifiers = 0; // USB HID modifier bits
    uint8_t keys[6] = {};  // 6KRO usages; 0 = empty slot
    uint16_t seq = 0;

    void clear() {
        modifiers = 0;
        for (auto &k : keys) {
            k = 0;
        }
    }

    bool anyKeyDown() const {
        if (modifiers != 0) {
            return true;
        }
        for (uint8_t k : keys) {
            if (k != 0) {
                return true;
            }
        }
        return false;
    }
};

struct MouseState {
    int8_t dx = 0;
    int8_t dy = 0;
    int8_t wheel = 0;
    int8_t pan = 0;
    uint8_t buttons = 0; // bit0 L, bit1 R, bit2 M
    uint16_t seq = 0;

    void clearMotion() {
        dx = dy = wheel = pan = 0;
    }

    void clearAll() {
        clearMotion();
        buttons = 0;
    }
};

struct ConsumerControlState {
    uint16_t usage = 0; // HID consumer usage, 0 = none
    uint16_t seq = 0;

    void clear() {
        usage = 0;
    }
};

enum class GamepadButton : uint32_t {
    A = 1u << 0,
    B = 1u << 1,
    X = 1u << 2,
    Y = 1u << 3,
    L1 = 1u << 4,
    R1 = 1u << 5,
    L2 = 1u << 6, // digital fallback
    R2 = 1u << 7,
    Select = 1u << 8,
    Start = 1u << 9,
    Home = 1u << 10,
    L3 = 1u << 11,
    R3 = 1u << 12,
    Extra1 = 1u << 13,
    Extra2 = 1u << 14,
    Extra3 = 1u << 15
};

enum class DpadDir : uint8_t {
    None = 0,
    Up = 1,
    UpRight = 2,
    Right = 3,
    DownRight = 4,
    Down = 5,
    DownLeft = 6,
    Left = 7,
    UpLeft = 8
};

struct GamepadState {
    int16_t leftX = 0;
    int16_t leftY = 0;
    int16_t rightX = 0;
    int16_t rightY = 0;
    uint8_t leftTrigger = 0;
    uint8_t rightTrigger = 0;
    uint32_t buttons = 0;
    DpadDir dpad = DpadDir::None;
    uint8_t batteryPercent = 0xFF; // 0xFF = unknown
    uint16_t seq = 0;

    bool button(GamepadButton b) const {
        return (buttons & static_cast<uint32_t>(b)) != 0;
    }

    void setButton(GamepadButton b, bool down) {
        const auto mask = static_cast<uint32_t>(b);
        if (down) {
            buttons |= mask;
        } else {
            buttons &= ~mask;
        }
    }

    void clear() {
        leftX = leftY = rightX = rightY = 0;
        leftTrigger = rightTrigger = 0;
        buttons = 0;
        dpad = DpadDir::None;
    }
};

struct NormalizedInput {
    enum class Kind : uint8_t { None, Keyboard, Mouse, Gamepad, Consumer };

    Kind kind = Kind::None;
    KeyboardState keyboard;
    MouseState mouse;
    GamepadState gamepad;
    ConsumerControlState consumer;
};

} // namespace blueshift
