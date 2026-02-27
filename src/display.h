#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

extern Adafruit_SSD1306 display;

void initDisplay();
void displayBMX(double ave, int MO, float cm, float mass, float magnitude);

#endif // DISPLAY_H