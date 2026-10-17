#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include "colors.h"

int main(void) {

    Display display;

    display.fill(Display::color565(255, 30, 30));
    display.update();

    sleep_ms(3000);
    display.fill(30, 255, 30);
    display.update(0, 0, 100, 100);

    sleep_ms(3000);
    display.fill(Display::color565(30, 30, 255));
    display.update(Display::WIDTH - 100, Display::HEIGHT - 100, 100, 100);

    return 0;
}