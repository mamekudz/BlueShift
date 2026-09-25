#pragma once

#include <cstddef>
#include <cstdint>

namespace blueshift {

// Project-owned display API. No ThingPulse/Adafruit types leak here.

class Display {
public:
    virtual ~Display() = default;
    virtual bool begin() = 0;
    virtual void clear() = 0;
    virtual void present() = 0;
    virtual void setContrast(uint8_t value) = 0; // 0..255; may map to on/off
    virtual void setPower(bool on) = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual void drawText(int x, int y, const char *text) = 0;
    virtual void drawBitmap(int x, int y, int w, int h, const uint8_t *bits) = 0;
    virtual void drawLine(int x0, int y0, int x1, int y1, bool on) = 0;
    virtual void fillRect(int x, int y, int w, int h, bool on) = 0;
    virtual void drawRect(int x, int y, int w, int h, bool on) = 0;
};

} // namespace blueshift
