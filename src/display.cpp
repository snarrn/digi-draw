// This Display class functions as a driver for Waveshare's Pico-ResTouch-LCD-2.8 product.
// Their product uses an ST7789 control chip, 240x320 resolution, and SPI1 communication interface.
#include "display.h"
#include "font.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

namespace {

// Pin map for Pico-ResTouch-LCD-2.8
constexpr uint PIN_DC   =  8;
constexpr uint PIN_CS   =  9;
constexpr uint PIN_CLK  = 10;
constexpr uint PIN_MOSI = 11;
constexpr uint PIN_BL   = 13;
constexpr uint PIN_RST  = 15;

#define LCD_SPI spi1

constexpr uint32_t SPI_BAUD = 62'500'000;

// ST7789 commands
constexpr uint8_t CMD_SLPOUT = 0x11;
constexpr uint8_t CMD_DISPON = 0x29;
constexpr uint8_t CMD_CASET  = 0x2A;
constexpr uint8_t CMD_RASET  = 0x2B;
constexpr uint8_t CMD_RAMWR  = 0x2C;
constexpr uint8_t CMD_MADCTL = 0x36;

// MADCTL: MX=1, MV=1 -> 320x240 landscape for Waveshare's U2D_R2L
constexpr uint8_t MADCTL_LANDSCAPE = 0x60;

// Initialization table: { cmd, nArgs, args }, terminated by cmd == 0x00.
// These values were derived from reference materials for the ST7789VW controller and Waveshare's Pico-ResTouch-LCD-2.8 documentation at https://www.waveshare.com/wiki/Pico-ResTouch-LCD-2.8.
constexpr uint8_t INIT_TABLE[] = {
    CMD_MADCTL, 1, MADCTL_LANDSCAPE,
    0x3A, 1, 0x55,                                  // 16-bit/pixel (RGB565)
    0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33,          // porch control
    0xB7, 1, 0x35,                                  // gate control
    0xBB, 1, 0x28,                                  // VCOM
    0xC0, 1, 0x3C,                                  // LCM control
    0xC2, 1, 0x01,                                  // VDV/VRH enable
    0xC3, 1, 0x0B,                                  // VRH
    0xC4, 1, 0x20,                                  // VDV
    0xC6, 1, 0x0F,                                  // frame rate
    0xD0, 2, 0xA4, 0xA1,                            // power control
    0xE0, 14, 0xD0, 0x01, 0x08, 0x0F, 0x11, 0x2A, 0x36,
        0x55, 0x44, 0x3A, 0x0B, 0x06, 0x11, 0x20,   // positive gamma
    0xE1, 14, 0xD0, 0x02, 0x07, 0x0A, 0x0B, 0x18, 0x34,
        0x43, 0x4A, 0x2B, 0x1B, 0x1C, 0x22, 0x1F,   // negative gamma
    0x55, 1, 0xB0,                                  // content adaptive brightness
    0x00
};

// 32-bit access to the 16-bit pixel buffer without violating strict aliasing.
typedef uint32_t __attribute__((may_alias)) u32_alias;

// Fill n pixels with a color using 32-bit stores where possible.
inline void fillSpan(uint16_t* p, int n, uint16_t c) {
    if (n <= 0) return;
    if (reinterpret_cast<uintptr_t>(p) & 2) { *p++ = c; --n; }
    const uint32_t v = (uint32_t(c) << 16) | c;
    u32_alias* q = reinterpret_cast<u32_alias*>(p);
    for (int words = n >> 1; words > 0; --words) *q++ = v;
    if (n & 1) *reinterpret_cast<uint16_t*>(q) = c;
}

inline void waitSpiIdle() {
    while (spi_get_hw(LCD_SPI)->sr & SPI_SSPSR_BSY_BITS) tight_loop_contents();
}

inline void csLow()  { gpio_put(PIN_CS, 0); }
inline void csHigh() { gpio_put(PIN_CS, 1); }

// Command/data helpers. All assume SPI is in 8-bit mode and CS is already low.
inline void writeCommand(uint8_t cmd) {
    gpio_put(PIN_DC, 0);
    spi_write_blocking(LCD_SPI, &cmd, 1);
    waitSpiIdle();                       // DC must not change until the byte has left the shifter
    gpio_put(PIN_DC, 1);
}

inline void writeData(const uint8_t* data, size_t len) {
    spi_write_blocking(LCD_SPI, data, len);
    waitSpiIdle();
}

void writeCommand(uint8_t cmd, const uint8_t* args, size_t n) {
    writeCommand(cmd);
    if (n) writeData(args, n);
}

// Inclusive window, then RAMWR. CS must be low, SPI in 8-bit mode.
void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t col[4] = { uint8_t(x0 >> 8), uint8_t(x0), uint8_t(x1 >> 8), uint8_t(x1) };
    uint8_t row[4] = { uint8_t(y0 >> 8), uint8_t(y0), uint8_t(y1 >> 8), uint8_t(y1) };
    writeCommand(CMD_CASET, col, 4);
    writeCommand(CMD_RASET, row, 4);
    writeCommand(CMD_RAMWR);
}

} // End of empty namespace

uint16_t Display::color565(uint8_t r, uint8_t g, uint8_t b) {
    return uint16_t(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

Display::Display() {
    // Keep the backlight off from the very start
    gpio_init(PIN_BL);
    gpio_set_dir(PIN_BL, GPIO_OUT);
    gpio_put(PIN_BL, 0);

    // --- GPIO / SPI ---------------------------------------------------------
    spi_init(LCD_SPI, SPI_BAUD);
    gpio_set_function(PIN_CLK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    for (uint pin : {PIN_DC, PIN_CS, PIN_RST}) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }
    csHigh();
    gpio_put(PIN_DC, 1);

    // Backlight PWM (stays off until the first frame is on the panel)
    gpio_set_function(PIN_BL, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_BL);
    pwm_set_wrap(slice, 255);
    pwm_set_gpio_level(PIN_BL, 0);
    pwm_set_enabled(slice, true);

    // --- DMA ----------------------------------------------------------------
    dmaChannel_ = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dmaChannel_);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_dreq(&c, spi_get_dreq(LCD_SPI, true));
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    dma_channel_configure(dmaChannel_, &c, &spi_get_hw(LCD_SPI)->dr, buffer, 0, false);

    // --- Panel reset + init -------------------------------------------------
    gpio_put(PIN_RST, 1); sleep_ms(5);
    gpio_put(PIN_RST, 0); sleep_ms(20);
    gpio_put(PIN_RST, 1); sleep_ms(120);

    csLow();
    writeCommand(CMD_SLPOUT);
    csHigh();
    sleep_ms(120);

    csLow();
    for (const uint8_t* p = INIT_TABLE; *p; ) {
        uint8_t cmd = *p++;
        uint8_t n   = *p++;
        writeCommand(cmd, p, n);
        p += n;
    }
    csHigh();                      // display stays off for now

    // Push a black frame while the panel is still off
    fill(uint16_t(0));
    update();

    // Now turn the panel on, then the backlight
    csLow();
    writeCommand(CMD_DISPON);
    csHigh();
    sleep_ms(20);
    setBacklightBrightness(255);
}

void Display::setBacklightBrightness(int brightness) {
    pwm_set_gpio_level(PIN_BL, uint16_t(std::clamp(brightness, 0, 255)));
}

void Display::update() {
    update(0, 0, WIDTH, HEIGHT);
}

void Display::update(int x, int y, int w, int h) {
    // Clip to screen
    int x1 = std::min(x + w, int(WIDTH));
    int y1 = std::min(y + h, int(HEIGHT));
    x = std::max(x, 0);
    y = std::max(y, 0);
    if (x >= x1 || y >= y1) return;
    w = x1 - x;
    h = y1 - y;

    csLow();
    setWindow(x, y, x1 - 1, y1 - 1);

    // Pixel data: 16-bit SPI frames send each uint16_t MSB first, which is exactly
    // the ST7789's RGB565 byte order, so no byte swapping is needed.
    spi_set_format(LCD_SPI, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    if (w == WIDTH) {
        // Rows are contiguous in the buffer: one DMA transfer for the whole region.
        dma_channel_set_trans_count(dmaChannel_, w * h, false);
        dma_channel_set_read_addr(dmaChannel_, &buffer[y * WIDTH], true);
        dma_channel_wait_for_finish_blocking(dmaChannel_);
    } else {
        // Sub-rectangle: one short DMA transfer per row.
        for (int row = y; row < y1; ++row) {
            dma_channel_set_trans_count(dmaChannel_, w, false);
            dma_channel_set_read_addr(dmaChannel_, &buffer[row * WIDTH + x], true);
            dma_channel_wait_for_finish_blocking(dmaChannel_);
        }
    }

    waitSpiIdle();  // DMA is done once the FIFO is fed; wait for the last bits to shift out
    csHigh();
    spi_set_format(LCD_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void Display::fill(uint16_t color) {
    // 32 bits at a time, unrolled. Requires 4-byte buffer alignment (alignas(4) in the header).
    static_assert((WIDTH * HEIGHT) % 8 == 0, "buffer size must be a multiple of 8 pixels");
    const uint32_t v = (uint32_t(color) << 16) | color;
    u32_alias* p = reinterpret_cast<u32_alias*>(buffer);
    u32_alias* const end = p + (WIDTH * HEIGHT) / 2;
    while (p < end) {
        p[0] = v; p[1] = v; p[2] = v; p[3] = v;
        p += 4;
    }
}

void Display::fill(uint8_t r, uint8_t g, uint8_t b) {
    fill(color565(r, g, b));
}

void Display::drawPixel(int x, int y, uint16_t color) {
    if (unsigned(x) < WIDTH && unsigned(y) < HEIGHT) buffer[y * WIDTH + x] = color;
}

uint16_t Display::getPixel(int x, int y) const {
    if (unsigned(x) < WIDTH && unsigned(y) < HEIGHT) return buffer[y * WIDTH + x];
    return 0;
}

void Display::hLine(int x, int y, int w, uint16_t color) {
    if (unsigned(y) >= HEIGHT || w <= 0) return;
    int x0 = std::max(x, 0);
    int x1 = std::min(x + w, int(WIDTH));
    if (x0 >= x1) return;
    fillSpan(&buffer[y * WIDTH + x0], x1 - x0, color);
}

void Display::vLine(int x, int y, int h, uint16_t color) {
    if (unsigned(x) >= WIDTH || h <= 0) return;
    int y0 = std::max(y, 0);
    int y1 = std::min(y + h, int(HEIGHT));
    if (y0 >= y1) return;
    uint16_t* p = &buffer[y0 * WIDTH + x];
    for (int row = y0; row < y1; ++row, p += WIDTH) *p = color;
}

void Display::fillRect(int x, int y, int w, int h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    int x0 = std::max(x, 0);
    int y0 = std::max(y, 0);
    int x1 = std::min(x + w, int(WIDTH));
    int y1 = std::min(y + h, int(HEIGHT));
    if (x0 >= x1 || y0 >= y1) return;
    for (int row = y0; row < y1; ++row) {
        fillSpan(&buffer[row * WIDTH + x0], x1 - x0, color);
    }
}

void Display::drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
    // Fast paths
    if (y0 == y1) {
        if (x0 > x1) std::swap(x0, x1);
        hLine(x0, y0, x1 - x0 + 1, color);
        return;
    }
    if (x0 == x1) {
        if (y0 > y1) std::swap(y0, y1);
        vLine(x0, y0, y1 - y0 + 1, color);
        return;
    }

    // Bresenham. Skip per-pixel bounds checks when both endpoints are on screen.
    const bool inside = unsigned(x0) < WIDTH && unsigned(x1) < WIDTH &&
                        unsigned(y0) < HEIGHT && unsigned(y1) < HEIGHT;
    const int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        if (inside) buffer[y0 * WIDTH + x0] = color;
        else        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void Display::drawRect(int x, int y, int w, int h, uint16_t color, bool filled) {
    if (w <= 0 || h <= 0) return;
    if (filled) {
        fillRect(x, y, w, h, color);
        return;
    }
    hLine(x, y, w, color);
    if (h > 1) hLine(x, y + h - 1, w, color);
    if (h > 2) {
        vLine(x, y + 1, h - 2, color);
        if (w > 1) vLine(x + w - 1, y + 1, h - 2, color);
    }
}

void Display::drawCircle(int cx, int cy, int r, uint16_t color, bool filled) {
    if (r < 0) return;
    if (r == 0) { drawPixel(cx, cy, color); return; }

    // Midpoint circle: walk one octant, mirror into the other seven.
    int px = 0, py = r, d = 1 - r;
    while (px <= py) {
        if (filled) {
            // Wide rows at y +/- px
            hLine(cx - py, cy + px, 2 * py + 1, color);
            if (px) hLine(cx - py, cy - px, 2 * py + 1, color);
            // Narrow rows at y +/- py
            if (px != py) {
                hLine(cx - px, cy + py, 2 * px + 1, color);
                hLine(cx - px, cy - py, 2 * px + 1, color);
            }
        } else {
            drawPixel(cx + px, cy + py, color);
            drawPixel(cx - px, cy + py, color);
            drawPixel(cx + px, cy - py, color);
            drawPixel(cx - px, cy - py, color);
            drawPixel(cx + py, cy + px, color);
            drawPixel(cx - py, cy + px, color);
            drawPixel(cx + py, cy - px, color);
            drawPixel(cx - py, cy - px, color);
        }
        ++px;
        if (d < 0) {
            d += 2 * px + 1;
        } else {
            --py;
            d += 2 * (px - py) + 1;
        }
    }
}

void Display::drawEllipse(int cx, int cy, int rx, int ry, uint16_t color, bool filled) {
    if (rx < 0 || ry < 0) return;
    if (rx == 0 && ry == 0) { drawPixel(cx, cy, color); return; }
    if (ry == 0) { hLine(cx - rx, cy, 2 * rx + 1, color); return; }
    if (rx == 0) { vLine(cx, cy - ry, 2 * ry + 1, color); return; }

    auto plot = [&](int x, int y) {
        if (filled) {
            hLine(cx - x, cy + y, 2 * x + 1, color);
            if (y) hLine(cx - x, cy - y, 2 * x + 1, color);
        } else {
            drawPixel(cx + x, cy + y, color);
            drawPixel(cx - x, cy + y, color);
            if (y) {
                drawPixel(cx + x, cy - y, color);
                drawPixel(cx - x, cy - y, color);
            }
        }
    };

    // Midpoint ellipse (64-bit math: rx^2 * ry^2 overflows 32 bits at screen sizes).
    const int64_t rx2 = int64_t(rx) * rx;
    const int64_t ry2 = int64_t(ry) * ry;
    int64_t x = 0, y = ry;
    int64_t px = 0, py = 2 * rx2 * y;

    // Region 1: slope magnitude < 1, step in x
    int64_t p = ry2 - rx2 * ry + rx2 / 4;
    while (px < py) {
        plot(int(x), int(y));
        ++x;
        px += 2 * ry2;
        if (p < 0) {
            p += ry2 + px;
        } else {
            --y;
            py -= 2 * rx2;
            p += ry2 + px - py;
        }
    }

    // Region 2: slope magnitude >= 1, step in y
    p = ry2 * (x * x + x) + ry2 / 4 + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
    while (y >= 0) {
        plot(int(x), int(y));
        --y;
        py -= 2 * rx2;
        if (p > 0) {
            p += rx2 - py;
        } else {
            ++x;
            px += 2 * ry2;
            p += rx2 - py + px;
        }
    }
}

void Display::drawPolygon(const Point* pts, int count, uint16_t color, bool filled) {
    if (!pts || count < 1 || count > MAX_POLYGON_VERTICES) return;
    if (count == 1) { drawPixel(pts[0].x, pts[0].y, color); return; }

    if (filled && count >= 3) {
        int yMin = pts[0].y, yMax = pts[0].y;
        for (int i = 1; i < count; ++i) {
            yMin = std::min(yMin, pts[i].y);
            yMax = std::max(yMax, pts[i].y);
        }
        yMin = std::max(yMin, 0);
        yMax = std::min(yMax, int(HEIGHT) - 1);

        int xs[MAX_POLYGON_VERTICES];
        for (int y = yMin; y <= yMax; ++y) {
            // Collect x crossings of this scanline (even-odd rule)
            int n = 0;
            for (int i = 0, j = count - 1; i < count; j = i++) {
                const int yi = pts[i].y, yj = pts[j].y;
                if ((yi <= y && yj > y) || (yj <= y && yi > y)) {
                    xs[n++] = pts[i].x + (y - yi) * (pts[j].x - pts[i].x) / (yj - yi);
                }
            }
            // Insertion sort (n is small)
            for (int a = 1; a < n; ++a) {
                int v = xs[a], b = a - 1;
                while (b >= 0 && xs[b] > v) { xs[b + 1] = xs[b]; --b; }
                xs[b + 1] = v;
            }
            for (int k = 0; k + 1 < n; k += 2) {
                hLine(xs[k], y, xs[k + 1] - xs[k], color);
            }
        }
        // Fall through: the outline closes the edges the half-open spans leave out.
    }

    for (int i = 0, j = count - 1; i < count; j = i++) {
        drawLine(pts[j].x, pts[j].y, pts[i].x, pts[i].y, color);
    }
}

void Display::drawChar(int x, int y, char ch, uint16_t color, int scale, int32_t bgColor, const Font& font) {
    const int cellW = font.cellWidth() * scale;
    const int cellH = font.cellHeight() * scale;
    if (x >= WIDTH || y >= HEIGHT || x + cellW <= 0 || y + cellH <= 0) return;
 
    if (bgColor >= 0) fillRect(x, y, cellW, cellH, uint16_t(bgColor));
 
    // Characters outside the font's range (including bytes >= 128) use unknownChar.
    unsigned c = static_cast<unsigned char>(ch);
    if (c < font.firstChar || c > font.lastChar) c = font.unknownChar;
    if (c < font.firstChar || c > font.lastChar) return;   // font has no usable fallback
    const int bpc = font.bytesPerColumn();
    const uint8_t* glyph = font.data + (c - font.firstChar) * font.bytesPerGlyph();
 
    for (int col = 0; col < font.width; ++col) {
        const uint8_t* colBytes = glyph + col * bpc;
        const int px = x + col * scale;
        if (px >= WIDTH) break;                 // everything further right is off screen
        int row = 0;
        while (row < font.height) {
            if (!((colBytes[row >> 3] >> (row & 7)) & 1)) { ++row; continue; }
            const int start = row;
            while (row < font.height && ((colBytes[row >> 3] >> (row & 7)) & 1)) ++row;  // merge vertical runs
            const int py = y + start * scale;
            if (scale == 1) {
                if (row - start == 1) drawPixel(px, py, color);
                else                  vLine(px, py, row - start, color);
            } else {
                fillRect(px, py, scale, (row - start) * scale, color);
            }
        }
    }
}
 
void Display::drawText(int x, int y, const char* text, uint16_t color, int scale,
                       int32_t bgColor, const Font& font) {
    if (!text || scale < 1) return;
    int cx = x, cy = y;
    for (const char* s = text; *s; ++s) {
        if (*s == '\n') { cx = x; cy += font.cellHeight() * scale; continue; }
        if (*s == '\r') continue;
        drawChar(cx, cy, *s, color, scale, bgColor, font);
        cx += font.cellWidth() * scale;
    }
}