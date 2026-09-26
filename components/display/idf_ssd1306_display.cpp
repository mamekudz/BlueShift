#if defined(ESP_PLATFORM) && !defined(ARDUINO)

#include "display/idf_ssd1306_display.h"

#include <cstring>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"

namespace blueshift {
namespace {

// 5x7 ASCII subset (32..127) — compact, no glyph framework.
// Rows are vertical bits for column-major SSD1306 pages when drawn via setPixel.
constexpr uint8_t kFont5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x00, 0x08, 0x14, 0x22, 0x41}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
};

const uint8_t *glyph(char c) {
    if (c < 32 || c > 90) {
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        } else {
            c = '?';
        }
    }
    return kFont5x7[static_cast<size_t>(c - 32)];
}

} // namespace

IdfSsd1306Display::IdfSsd1306Display(const t_lion::BoardConfig &cfg) : cfg_(cfg) {}

IdfSsd1306Display::~IdfSsd1306Display() {
    if (devHandle_ != nullptr) {
        i2c_master_bus_rm_device(static_cast<i2c_master_dev_handle_t>(devHandle_));
        devHandle_ = nullptr;
    }
    if (busHandle_ != nullptr) {
        i2c_del_master_bus(static_cast<i2c_master_bus_handle_t>(busHandle_));
        busHandle_ = nullptr;
    }
}

bool IdfSsd1306Display::sendCommand(uint8_t cmd) {
    if (devHandle_ == nullptr) {
        return false;
    }
    uint8_t buf[2] = {0x00, cmd};
    return i2c_master_transmit(static_cast<i2c_master_dev_handle_t>(devHandle_), buf, 2, 100) ==
           ESP_OK;
}

bool IdfSsd1306Display::sendCommands(const uint8_t *cmds, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (!sendCommand(cmds[i])) {
            return false;
        }
    }
    return true;
}

bool IdfSsd1306Display::sendFramebuffer() {
    if (devHandle_ == nullptr) {
        return false;
    }
    // Control 0x40 = data stream.
    uint8_t packet[1 + 128];
    for (int page = 0; page < 8; ++page) {
        if (!sendCommand(0xB0 | page) || !sendCommand(0x00) || !sendCommand(0x10)) {
            return false;
        }
        packet[0] = 0x40;
        std::memcpy(packet + 1, fb_ + page * 128, 128);
        if (i2c_master_transmit(static_cast<i2c_master_dev_handle_t>(devHandle_), packet,
                                sizeof(packet), 100) != ESP_OK) {
            return false;
        }
    }
    return true;
}

bool IdfSsd1306Display::begin() {
    i2c_master_bus_config_t busCfg = {};
    busCfg.i2c_port = I2C_NUM_0;
    busCfg.sda_io_num = static_cast<gpio_num_t>(cfg_.oledSda);
    busCfg.scl_io_num = static_cast<gpio_num_t>(cfg_.oledScl);
    busCfg.clk_source = I2C_CLK_SRC_DEFAULT;
    busCfg.glitch_ignore_cnt = 7;
    busCfg.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus = nullptr;
    if (i2c_new_master_bus(&busCfg, &bus) != ESP_OK) {
        return false;
    }
    busHandle_ = bus;

    i2c_device_config_t devCfg = {};
    devCfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    devCfg.device_address = cfg_.oledAddress;
    devCfg.scl_speed_hz = 400000;

    i2c_master_dev_handle_t dev = nullptr;
    if (i2c_master_bus_add_device(bus, &devCfg, &dev) != ESP_OK) {
        return false;
    }
    devHandle_ = dev;

    static const uint8_t kInit[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA,
        0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF,
    };
    if (!sendCommands(kInit, sizeof(kInit))) {
        return false;
    }
    clear();
    present();
    ready_ = true;
    return true;
}

void IdfSsd1306Display::clear() {
    std::memset(fb_, 0, sizeof(fb_));
}

void IdfSsd1306Display::present() {
    if (ready_ && powerOn_) {
        sendFramebuffer();
    }
}

void IdfSsd1306Display::setContrast(uint8_t value) {
    sendCommand(0x81);
    sendCommand(value);
}

void IdfSsd1306Display::setPower(bool on) {
    powerOn_ = on;
    sendCommand(on ? 0xAF : 0xAE);
}

int IdfSsd1306Display::width() const {
    return cfg_.oledWidth;
}

int IdfSsd1306Display::height() const {
    return cfg_.oledHeight;
}

void IdfSsd1306Display::setPixel(int x, int y, bool on) {
    if (x < 0 || y < 0 || x >= cfg_.oledWidth || y >= cfg_.oledHeight) {
        return;
    }
    const int index = x + (y / 8) * cfg_.oledWidth;
    const uint8_t mask = static_cast<uint8_t>(1u << (y & 7));
    if (on) {
        fb_[index] = static_cast<uint8_t>(fb_[index] | mask);
    } else {
        fb_[index] = static_cast<uint8_t>(fb_[index] & static_cast<uint8_t>(~mask));
    }
}

void IdfSsd1306Display::drawText(int x, int y, const char *text) {
    if (text == nullptr) {
        return;
    }
    int cx = x;
    for (const char *p = text; *p != '\0'; ++p) {
        const uint8_t *g = glyph(*p);
        for (int col = 0; col < 5; ++col) {
            const uint8_t bits = g[col];
            for (int row = 0; row < 7; ++row) {
                setPixel(cx + col, y + row, (bits & (1u << row)) != 0);
            }
        }
        cx += 6;
    }
}

void IdfSsd1306Display::drawBitmap(int x, int y, int w, int h, const uint8_t *bits) {
    if (bits == nullptr) {
        return;
    }
    for (int row = 0; row < h; ++row) {
        for (int col = 0; col < w; ++col) {
            const int byteIndex = row * ((w + 7) / 8) + (col / 8);
            const bool on = (bits[byteIndex] & (0x80 >> (col & 7))) != 0;
            setPixel(x + col, y + row, on);
        }
    }
}

void IdfSsd1306Display::drawLine(int x0, int y0, int x1, int y1, bool on) {
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = x0 < x1 ? 1 : -1;
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        setPixel(x0, y0, on);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void IdfSsd1306Display::fillRect(int x, int y, int w, int h, bool on) {
    for (int yy = y; yy < y + h; ++yy) {
        for (int xx = x; xx < x + w; ++xx) {
            setPixel(xx, yy, on);
        }
    }
}

void IdfSsd1306Display::drawRect(int x, int y, int w, int h, bool on) {
    drawLine(x, y, x + w - 1, y, on);
    drawLine(x, y + h - 1, x + w - 1, y + h - 1, on);
    drawLine(x, y, x, y + h - 1, on);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, on);
}

} // namespace blueshift

#endif
