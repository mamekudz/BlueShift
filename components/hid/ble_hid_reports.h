#pragma once

#include <cstddef>
#include <cstdint>

namespace blueshift {

// Standards-oriented BLE HID report builders for host tests.
// Separate descriptors — avoid unjustified mega-composite.

struct HidDescriptorView {
    const uint8_t *data;
    std::size_t size;
};

HidDescriptorView bleKeyboardDescriptor();
HidDescriptorView bleMouseDescriptor();
HidDescriptorView bleGamepadDescriptor();

// Boot keyboard: modifiers + reserved + 6 keys = 8 bytes.
bool buildKeyboardReport(uint8_t modifiers, const uint8_t keys[6], uint8_t out[8]);

// Mouse: buttons, dx, dy, wheel = 4 bytes.
bool buildMouseReport(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, uint8_t out[4]);

// Gamepad: lx,ly,rx,ry, lt, rt, buttons_lo, buttons_hi, hat = 9 bytes (example layout).
bool buildGamepadReport(int16_t lx, int16_t ly, int16_t rx, int16_t ry, uint8_t lt, uint8_t rt,
                        uint16_t buttons, uint8_t hat, uint8_t out[9]);

uint8_t scaleAxisToUint8(int16_t axis);

} // namespace blueshift
