#include "hid/device_parsers.h"

#include <cstring>

namespace blueshift {

bool GenericKeyboardParser::parse(const RawHidReport &raw, KeyboardState &out) const {
    out.clear();
    if (raw.data == nullptr || raw.length == 0) {
        return false;
    }
    // Reject oversized / truncated before copy.
    if (raw.length > 64) {
        return false;
    }
    const uint8_t *p = raw.data;
    std::size_t n = raw.length;
    if (n == 9 && raw.reportId != 0) {
        // Some stacks prepend report id separately; if length 9, skip first byte.
        p += 1;
        n -= 1;
    }
    if (n < 8) {
        return false;
    }
    out.modifiers = p[0];
    for (int i = 0; i < 6; ++i) {
        out.keys[i] = p[2 + i];
    }
    return true;
}

bool GenericMouseParser::parse(const RawHidReport &raw, MouseState &out) const {
    out.clearAll();
    if (raw.data == nullptr || raw.length < 3 || raw.length > 8) {
        return false;
    }
    out.buttons = raw.data[0] & 0x07;
    out.dx = static_cast<int8_t>(raw.data[1]);
    out.dy = static_cast<int8_t>(raw.data[2]);
    if (raw.length >= 4) {
        out.wheel = static_cast<int8_t>(raw.data[3]);
    }
    return true;
}

bool Sn30ProProfile::matchesVidPid(uint16_t vid, uint16_t /*pid*/) const {
    return vid == kVid;
}

bool Sn30ProProfile::parseGamepad(const RawHidReport &raw, GamepadState &out) const {
    out.clear();
    if (raw.data == nullptr) {
        return false;
    }
    // Bluepad32 / community dumps suggest packed digital+analog layouts that vary by mode.
    // Until physical capture on BlueShift hardware, only accept a conservative 8+ byte layout
    // used by several 8BitDo Classic modes (DOCUMENTED in open parsers — UNVERIFIED here):
    // byte0 buttons lo, byte1 buttons hi, byte2 hat/dpad, byte3-6 axes (uint8 centered 0x80).
    if (raw.length < 7 || raw.length > 32) {
        return false;
    }
    const uint8_t *p = raw.data;
    const uint16_t buttons = static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
    // Map only bits that align with common 8BitDo digital maps (A/B/X/Y/L/R/Select/Start).
    if (buttons & 0x0001) {
        out.setButton(GamepadButton::A, true);
    }
    if (buttons & 0x0002) {
        out.setButton(GamepadButton::B, true);
    }
    if (buttons & 0x0008) {
        out.setButton(GamepadButton::X, true);
    }
    if (buttons & 0x0010) {
        out.setButton(GamepadButton::Y, true);
    }
    if (buttons & 0x0040) {
        out.setButton(GamepadButton::L1, true);
    }
    if (buttons & 0x0080) {
        out.setButton(GamepadButton::R1, true);
    }
    if (buttons & 0x0100) {
        out.setButton(GamepadButton::Select, true);
    }
    if (buttons & 0x0200) {
        out.setButton(GamepadButton::Start, true);
    }
    const uint8_t hat = p[2] & 0x0F;
    if (hat <= 8) {
        out.dpad = static_cast<DpadDir>(hat);
    }
    auto axisFromU8 = [](uint8_t v) -> int16_t {
        const int centered = static_cast<int>(v) - 128;
        return static_cast<int16_t>(centered * 256);
    };
    if (raw.length >= 7) {
        out.leftX = axisFromU8(p[3]);
        out.leftY = axisFromU8(p[4]);
        out.rightX = axisFromU8(p[5]);
        out.rightY = axisFromU8(p[6]);
    }
    return true;
}

bool Nimbus69070Profile::parseGamepad(const RawHidReport &raw, GamepadState &out) const {
    out.clear();
    if (raw.data == nullptr || raw.length < 4 || raw.length > 32) {
        return false;
    }
    // Evidence gap: no verified Classic report layout in-repo.
    // Accept only a minimal digital-button first byte if present — axes UNKNOWN.
    // IMPLEMENTED_UNVERIFIED / incomplete by design.
    const uint8_t b = raw.data[0];
    if (b & 0x01) {
        out.setButton(GamepadButton::A, true);
    }
    if (b & 0x02) {
        out.setButton(GamepadButton::B, true);
    }
    if (b & 0x04) {
        out.setButton(GamepadButton::X, true);
    }
    if (b & 0x08) {
        out.setButton(GamepadButton::Y, true);
    }
    return true;
}

} // namespace blueshift
