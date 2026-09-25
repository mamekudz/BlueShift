#pragma once

#include <cstddef>
#include <cstdint>

#include "hid/normalized_hid.h"

namespace blueshift {

// Generic Classic HID keyboard parser (boot-style 8-byte reports).
// Evidence: USB HID boot keyboard layout is industry-standard.

struct RawHidReport {
    uint8_t reportId = 0;
    const uint8_t *data = nullptr;
    std::size_t length = 0;
};

class GenericKeyboardParser {
public:
    // Accepts 8-byte boot keyboard (mod, reserved, 6 keys) or with leading report id.
    bool parse(const RawHidReport &raw, KeyboardState &out) const;
};

class GenericMouseParser {
public:
    bool parse(const RawHidReport &raw, MouseState &out) const;
};

// Evidence-backed fields only. Unknown bits left clear.
class Sn30ProProfile {
public:
    static constexpr uint16_t kVid = 0x2DC8; // 8BitDo — DOCUMENTED in public VID lists / Bluepad32
    // PID varies by firmware/mode — do not hard-bind a single PID as sole identity.

    bool matchesVidPid(uint16_t vid, uint16_t pid) const;
    bool parseGamepad(const RawHidReport &raw, GamepadState &out) const;
};

class Nimbus69070Profile {
public:
    // SteelSeries Nimbus — Classic BR/EDR candidate. Exact report map TBD.
    bool parseGamepad(const RawHidReport &raw, GamepadState &out) const;
};

} // namespace blueshift
