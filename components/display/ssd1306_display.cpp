#if defined(ARDUINO) && defined(BLUESHIFT_BOARD_T_LION)

#include "display/ssd1306_display.h"

#include <SSD1306Wire.h>
#include <Wire.h>

namespace blueshift {

struct Ssd1306Display::Impl {
    SSD1306Wire *oled = nullptr;
};

Ssd1306Display::Ssd1306Display(const t_lion::BoardConfig &cfg) : impl_(new Impl), cfg_(cfg) {}

Ssd1306Display::~Ssd1306Display() {
    if (impl_ != nullptr) {
        delete impl_->oled;
        delete impl_;
        impl_ = nullptr;
    }
}

bool Ssd1306Display::begin() {
    if (impl_->oled == nullptr) {
        impl_->oled = new SSD1306Wire(cfg_.oledAddress, cfg_.oledSda, cfg_.oledScl);
    }
    Wire.begin(cfg_.oledSda, cfg_.oledScl);
    const bool ok = impl_->oled->init();
    if (ok) {
        impl_->oled->flipScreenVertically();
        impl_->oled->setFont(ArialMT_Plain_10);
        impl_->oled->setTextAlignment(TEXT_ALIGN_LEFT);
        clear();
        present();
    }
    return ok;
}

void Ssd1306Display::clear() {
    if (impl_->oled != nullptr) {
        impl_->oled->clear();
    }
}

void Ssd1306Display::present() {
    if (impl_->oled != nullptr) {
        impl_->oled->display();
    }
}

void Ssd1306Display::setContrast(uint8_t value) {
    if (impl_->oled != nullptr) {
        impl_->oled->setContrast(value);
    }
}

void Ssd1306Display::setPower(bool on) {
    if (impl_->oled != nullptr) {
        impl_->oled->displayOn();
        if (!on) {
            impl_->oled->displayOff();
        }
    }
}

int Ssd1306Display::width() const {
    return cfg_.oledWidth;
}

int Ssd1306Display::height() const {
    return cfg_.oledHeight;
}

void Ssd1306Display::drawText(int x, int y, const char *text) {
    if (impl_->oled != nullptr && text != nullptr) {
        impl_->oled->drawString(x, y, text);
    }
}

void Ssd1306Display::drawBitmap(int x, int y, int w, int h, const uint8_t *bits) {
    if (impl_->oled == nullptr || bits == nullptr) {
        return;
    }
    // ThingPulse expects width/height and XBM-style bits.
    impl_->oled->drawXbm(x, y, w, h, bits);
}

void Ssd1306Display::drawLine(int x0, int y0, int x1, int y1, bool on) {
    if (impl_->oled != nullptr && on) {
        impl_->oled->drawLine(x0, y0, x1, y1);
    }
}

void Ssd1306Display::fillRect(int x, int y, int w, int h, bool on) {
    if (impl_->oled != nullptr && on) {
        impl_->oled->fillRect(x, y, w, h);
    }
}

void Ssd1306Display::drawRect(int x, int y, int w, int h, bool on) {
    if (impl_->oled != nullptr && on) {
        impl_->oled->drawRect(x, y, w, h);
    }
}

} // namespace blueshift

#endif
