#pragma once

#include <cstring>

#include "display/display.h"

namespace blueshift {

// Host/mock 128x64 1-bit framebuffer (1024 bytes).

class FramebufferDisplay : public Display {
public:
    static constexpr int kW = 128;
    static constexpr int kH = 64;
    static constexpr int kBytes = (kW * kH) / 8;

    bool begin() override {
        clear();
        powered_ = true;
        return true;
    }

    void clear() override {
        std::memset(pages_, 0, sizeof(pages_));
    }

    void present() override {}

    void setContrast(uint8_t) override {}

    void setPower(bool on) override {
        powered_ = on;
        if (!on) {
            clear();
        }
    }

    int width() const override {
        return kW;
    }

    int height() const override {
        return kH;
    }

    void setPixel(int x, int y, bool on) {
        if (!powered_ || x < 0 || y < 0 || x >= kW || y >= kH) {
            return;
        }
        const int page = y / 8;
        const int bit = y % 8;
        const int index = page * kW + x;
        if (on) {
            pages_[index] = static_cast<uint8_t>(pages_[index] | (1u << bit));
        } else {
            pages_[index] = static_cast<uint8_t>(pages_[index] & ~(1u << bit));
        }
    }

    bool getPixel(int x, int y) const {
        if (x < 0 || y < 0 || x >= kW || y >= kH) {
            return false;
        }
        const int page = y / 8;
        const int bit = y % 8;
        return (pages_[page * kW + x] & (1u << bit)) != 0;
    }

    void fillRect(int x, int y, int w, int h, bool on) override {
        for (int yy = y; yy < y + h; ++yy) {
            for (int xx = x; xx < x + w; ++xx) {
                setPixel(xx, yy, on);
            }
        }
    }

    void drawRect(int x, int y, int w, int h, bool on) override {
        for (int xx = x; xx < x + w; ++xx) {
            setPixel(xx, y, on);
            setPixel(xx, y + h - 1, on);
        }
        for (int yy = y; yy < y + h; ++yy) {
            setPixel(x, yy, on);
            setPixel(x + w - 1, yy, on);
        }
    }

    void drawLine(int x0, int y0, int x1, int y1, bool on) override {
        int dx = x1 - x0;
        int dy = y1 - y0;
        const int sx = dx >= 0 ? 1 : -1;
        const int sy = dy >= 0 ? 1 : -1;
        dx = dx >= 0 ? dx : -dx;
        dy = dy >= 0 ? dy : -dy;
        int err = dx - dy;
        for (;;) {
            setPixel(x0, y0, on);
            if (x0 == x1 && y0 == y1) {
                break;
            }
            const int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    void drawBitmap(int x, int y, int w, int h, const uint8_t *bits) override {
        if (bits == nullptr) {
            return;
        }
        for (int yy = 0; yy < h; ++yy) {
            for (int xx = 0; xx < w; ++xx) {
                const int bitIndex = yy * w + xx;
                const bool on = (bits[bitIndex / 8] & (1u << (7 - (bitIndex % 8)))) != 0;
                if (on) {
                    setPixel(x + xx, y + yy, true);
                }
            }
        }
    }

    void drawText(int x, int y, const char *text) override {
        if (text == nullptr) {
            return;
        }
        int cx = x;
        for (const char *p = text; *p; ++p) {
            drawGlyph(cx, y, *p);
            cx += 6;
            if (cx >= kW) {
                break;
            }
        }
    }

    const uint8_t *pages() const {
        return pages_;
    }

    std::size_t litCount() const {
        std::size_t n = 0;
        for (int i = 0; i < kBytes; ++i) {
            uint8_t v = pages_[i];
            while (v) {
                n += v & 1u;
                v = static_cast<uint8_t>(v >> 1);
            }
        }
        return n;
    }

private:
    void drawGlyph(int x, int y, char c) {
        if (c <= 32) {
            return;
        }
        fillRect(x, y, 5, 7, true);
    }

    uint8_t pages_[kBytes] = {};
    bool powered_ = true;
};

} // namespace blueshift
