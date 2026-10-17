// Colors provides a several basic colors
#pragma once
#include <cstdint>

namespace Colors {                                 // Standard RGB values shown here
    inline constexpr uint16_t WHITE =      0xFFFF; // 255, 255, 255
    inline constexpr uint16_t LIGHT_GRAY = 0xD6BA; inline constexpr uint16_t LIGHT_GREY = 0xD6BA; // 215, 215, 215
    inline constexpr uint16_t GRAY =       0x94B2; inline constexpr uint16_t GREY =       0x94B2; // 150, 150, 150
    inline constexpr uint16_t DARK_GRAY =  0x2945; inline constexpr uint16_t DARK_GREY =  0x2945; //  40,  40,  40
    inline constexpr uint16_t BLACK =      0x0000; //   0,   0,   0

    inline constexpr uint16_t RED =        0xF966; // 255,  45,  55
    inline constexpr uint16_t ORANGE =     0xF426; // 245, 135,  55
    inline constexpr uint16_t YELLOW =     0xEEA3; // 235, 215,  30
    inline constexpr uint16_t GREEN =      0x96A5; // 150, 215,  45
    inline constexpr uint16_t CYAN =       0x4655; //  65, 200, 175
    inline constexpr uint16_t BLUE =       0x353E; //  50, 165, 245
    inline constexpr uint16_t DEEP_BLUE =  0x8A9F; // 140,  80, 255
    inline constexpr uint16_t PURPLE =     0xE15F; // 225,  40, 250
    inline constexpr uint16_t MAGENTA =    0xFB36; // 250, 100, 180
    inline constexpr uint16_t PINK =       0xF577; // 245, 175, 185
}