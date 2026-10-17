// This Display class functions as a driver for Waveshare's Pico-ResTouch-LCD-2.8 product.
// Their product uses an ST7789 control chip, 240x320 resolution, and SPI1 communication interface.
#include "Display.h"
#include "pico/stdlib.h"

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

Display::Display() { }