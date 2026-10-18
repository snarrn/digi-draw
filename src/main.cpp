#include <cstdint>

#include "pico/stdlib.h"
#include "display.h"
#include "colors.h"

uint16_t all_colors[] = {
    Colors::LIGHT_RED, Colors::RED, Colors::DARK_RED,
    Colors::LIGHT_ORANGE, Colors::ORANGE, Colors::DARK_ORANGE,
    Colors::LIGHT_YELLOW, Colors::YELLOW, Colors::DARK_YELLOW,
    Colors::LIGHT_GREEN, Colors::GREEN, Colors::DARK_GREEN,
    Colors::LIGHT_CYAN, Colors::CYAN, Colors::DARK_CYAN,
    Colors::LIGHT_BLUE, Colors::BLUE, Colors::DARK_BLUE,
    Colors::LIGHT_PURPLE, Colors::PURPLE, Colors::DARK_PURPLE,
    Colors::LIGHT_MAGENTA, Colors::MAGENTA, Colors::DARK_MAGENTA,
    Colors::LIGHT_PINK, Colors::PINK, Colors::DARK_PINK,
    Colors::WHITE, Colors::WHITE, Colors::WHITE,
    Colors::LIGHT_GRAY, Colors::LIGHT_GRAY, Colors::LIGHT_GRAY,
    Colors::GRAY, Colors::GRAY, Colors::GRAY,
    Colors::DARK_GRAY, Colors::DARK_GRAY, Colors::DARK_GRAY,
    Colors::BLACK, Colors::BLACK, Colors::BLACK
};

int main(void) {
    stdio_init_all();

    Display disp;
    disp.fill(0);

    const int spacing = 16;
    const int third = Display::WIDTH / 3;

    for (int i = 0; i < 42/3; i++) {
        int y = spacing * i;

        for (int j = 0; j < 3; j++) {
            int x = third * j;

            disp.drawRect(x, y, third+1, spacing, all_colors[i*3+2-j]);
        }
    }

    disp.update();

    return 0;
}