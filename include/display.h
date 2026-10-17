// This Display class functions as a driver for Waveshare's Pico-ResTouch-LCD-2.8 product.
// Their product uses an ST7789 control chip, 240x320 resolution, and SPI1 communication interface.
#pragma once
#include <cstdint>

class Display {
public:
    static constexpr uint16_t WIDTH = 320;
    static constexpr uint16_t HEIGHT = 240;

    static uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

    Display();

    void update();
    void update(int x, int y, int w, int h);
    void setBacklightBrightness(int brightness);

    void fill(uint16_t color);
    void fill(uint8_t r, uint8_t g, uint8_t b);

    void drawPixel(int x, int y, uint16_t color);
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
    void drawRect(int x, int y, int w, int h, uint16_t color, bool filled=true);
    void drawCircle(int x, int y, int r, uint16_t color, bool filled=true);
    void drawEllipse(int x, int y, int rx, int ry, uint16_t color, bool filled=true);
    // void drawPolygon(); Implement these functions later
    // void drawText();

    // void blit();
    // void scroll(int x, int y);
    uint16_t getPixel(int x, int y) const;

private:
    alignas(4) uint16_t buffer[WIDTH * HEIGHT] = {};
    int dmaChannel_ = -1;
};