#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace blueshift {

// Minimal display abstraction. Firmware will bind SSD1306; host tests use FramebufferDisplay.

class Display {
public:
    virtual ~Display() = default;
    virtual bool begin() = 0;
    virtual void clear() = 0;
    virtual void present() = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual void drawText(int x, int y, const char *text) = 0;
    virtual void fillRect(int x, int y, int w, int h, bool on) = 0;
    virtual void drawRect(int x, int y, int w, int h, bool on) = 0;
};

class FramebufferDisplay : public Display {
public:
    static constexpr int kW = 128;
    static constexpr int kH = 64;

    bool begin() override {
        clear();
        return true;
    }

    void clear() override {
        std::memset(pixels_, 0, sizeof(pixels_));
    }

    void present() override {}

    int width() const override {
        return kW;
    }

    int height() const override {
        return kH;
    }

    void setPixel(int x, int y, bool on) {
        if (x < 0 || y < 0 || x >= kW || y >= kH) {
            return;
        }
        const int index = y * kW + x;
        pixels_[index] = on ? 1 : 0;
    }

    bool getPixel(int x, int y) const {
        if (x < 0 || y < 0 || x >= kW || y >= kH) {
            return false;
        }
        return pixels_[y * kW + x] != 0;
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

    // 5x7 glyph subset for host tests / placeholder UI.
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

    std::size_t litCount() const {
        std::size_t n = 0;
        for (std::uint8_t v : pixels_) {
            n += v ? 1 : 0;
        }
        return n;
    }

private:
    void drawGlyph(int x, int y, char c) {
        // Block placeholder: draw a 5x7 filled rect for printable ASCII.
        if (c <= 32) {
            return;
        }
        fillRect(x, y, 5, 7, true);
    }

    std::uint8_t pixels_[kW * kH] = {};
};

} // namespace blueshift
