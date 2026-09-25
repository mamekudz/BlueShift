#pragma once

#include "display/display.h"
#include "hardware/t_lion/board.h"

namespace blueshift {

#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)

// SSD1306 via ThingPulse SSD1306Wire — IMPLEMENTED_UNVERIFIED.
class Ssd1306Display final : public Display {
public:
    explicit Ssd1306Display(const t_lion::BoardConfig &cfg);
    ~Ssd1306Display() override;

    bool begin() override;
    void clear() override;
    void present() override;
    void setContrast(uint8_t value) override;
    void setPower(bool on) override;
    int width() const override;
    int height() const override;
    void drawText(int x, int y, const char *text) override;
    void drawBitmap(int x, int y, int w, int h, const uint8_t *bits) override;
    void drawLine(int x0, int y0, int x1, int y1, bool on) override;
    void fillRect(int x, int y, int w, int h, bool on) override;
    void drawRect(int x, int y, int w, int h, bool on) override;

private:
    struct Impl;
    Impl *impl_;
    t_lion::BoardConfig cfg_;
};

#endif

} // namespace blueshift
