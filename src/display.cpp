// This Display class functions as a driver for Waveshare's Pico-ResTouch-LCD-2.8 product.
// Their product uses an ST7789 control chip, 240x320 resolution, and SPI1 communication interface.
#include "Display.h"
#include "pico/stdlib.h"

namespace {

// Pin map for Pico-ResTouch-LCD-2.8
constexpr uint PIN_DC   =  8;
constexpr uint PIN_CS   =  9;
constexpr uint PIN_CLK  = 10;
constexpr uint PIN_MOSI = 11;
constexpr uint PIN_BL   = 13;
constexpr uint PIN_RST  = 15;

#define LDC_SPI spi1

constexpr uint32_t SPI_BAUD = 6'2500'000;

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

}

Display::Display() { }