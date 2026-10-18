// Colors provides a several basic colors
#pragma once
#include <cstdint>

namespace Colors {                                 // Standard RGB values shown here
    inline constexpr uint16_t WHITE =      0xFFFF; // 255, 255, 255
    inline constexpr uint16_t LIGHT_GRAY = 0xD6BA; inline constexpr uint16_t LIGHT_GREY = 0xD6BA; // 215, 215, 215
    inline constexpr uint16_t GRAY =       0x94B2; inline constexpr uint16_t GREY =       0x94B2; // 150, 150, 150
    inline constexpr uint16_t DARK_GRAY =  0x2945; inline constexpr uint16_t DARK_GREY =  0x2945; //  40,  40,  40
    inline constexpr uint16_t BLACK =      0x0000; //   0,   0,   0

    inline constexpr uint16_t RED =        0xF906; // 255,  35,  55
    inline constexpr uint16_t ORANGE =     0xF426; // 245, 135,  55
    inline constexpr uint16_t YELLOW =     0xEEA3; // 235, 215,  30
    inline constexpr uint16_t GREEN =      0x96A5; // 150, 215,  45
    inline constexpr uint16_t CYAN =       0x4655; //  65, 200, 175
    inline constexpr uint16_t BLUE =       0x353E; //  50, 165, 245
    inline constexpr uint16_t PURPLE =     0xAA59; // 175,  75, 205
    inline constexpr uint16_t MAGENTA =    0xCA77; // 207,  76, 185
    inline constexpr uint16_t PINK =       0xF577; // 245, 175, 185

    inline constexpr uint16_t DARK_RED =        0xC925; // 204,  36,  44
    inline constexpr uint16_t DARK_ORANGE =     0xC365; // 196, 108,  44
    inline constexpr uint16_t DARK_YELLOW =     0xBD63; // 188, 172,  24
    inline constexpr uint16_t DARK_GREEN =      0x7D64; // 120, 172,  36
    inline constexpr uint16_t DARK_CYAN =       0x3511; //  52, 160, 140
    inline constexpr uint16_t DARK_BLUE =       0x7219; // 112,  64, 204
    inline constexpr uint16_t DARK_PURPLE =     0x9176; // 150,  45, 180
    inline constexpr uint16_t DARK_MAGENTA =    0xBA55; // 189,  72, 173
    inline constexpr uint16_t DARK_PINK =       0xD493; // 210, 145, 155

    inline constexpr uint16_t LIGHT_RED =       0xFAAB; // 255,  85,  95
    inline constexpr uint16_t LIGHT_ORANGE =    0xFD4A; // 255, 171,  84
    inline constexpr uint16_t LIGHT_YELLOW =    0xFFE4; // 255, 255,  36
    inline constexpr uint16_t LIGHT_GREEN =     0xB7E6; // 180, 255,  54
    inline constexpr uint16_t LIGHT_CYAN =      0x57D9; //  80, 250, 205
    inline constexpr uint16_t LIGHT_BLUE =      0x3E3F; //  60, 198, 255
    inline constexpr uint16_t LIGHT_PURPLE =    0xC33C; // 195, 100, 225
    inline constexpr uint16_t LIGHT_MAGENTA =   0xFBDB; // 255, 120, 216
    inline constexpr uint16_t LIGHT_PINK =      0xFE9B; // 255, 210, 222
}