// Font struct that describes the basics of a custom font
// All fonts are drawn monospaced, with every glyph of the same dimensions.
#pragma once
#include <cstdint>

struct Font {
    const uint8_t* data; // glyph bitmaps, packed one after another
    uint8_t width;       // width of glyph in pixels
    uint8_t height;      // height of glyph in pixels
    uint8_t firstChar;   // describes the lower bound of supported characters
    uint8_t lastChar;    // describes the upper bound of supported characters
    uint8_t unknownChar; // the character drawn for characters out of the supported range
};