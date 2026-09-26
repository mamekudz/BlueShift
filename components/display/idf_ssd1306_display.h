#pragma once

#include "display/display.h"
#include "hardware/t_lion/board.h"

namespace blueshift {

#if defined(ESP_PLATFORM) && !defined(ARDUINO)

// Project-owned SSD1306 128x64 I2C backend for native ESP-IDF.
// No ThingPulse / Arduino dependency. IMPLEMENTED_UNVERIFIED.
class IdfSsd1306Display final : public Display {
public:
    explicit IdfSsd1306Display(const t_lion::BoardConfig &cfg);
    ~IdfSsd1306Display() override;

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
    void setPixel(int x, int y, bool on);
    bool sendCommand(uint8_t cmd);
    bool sendCommands(const uint8_t *cmds, size_t len);
    bool sendFramebuffer();

    t_lion::BoardConfig cfg_;
    uint8_t fb_[1024]{};
    void *busHandle_ = nullptr;
    void *devHandle_ = nullptr;
    bool ready_ = false;
    bool powerOn_ = true;
};

#endif

} // namespace blueshift
