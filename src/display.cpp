// This Display class functions as a driver for Waveshare's Pico-ResTouch-LCD-2.8 product.
// Their product uses an ST7789 control chip, 240x320 resolution, and SPI1 communication interface.
#include "Display.h"
#include "pico/stdlib.h"

#include "hardware/spi.h"

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

Display::Display() { }