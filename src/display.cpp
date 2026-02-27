#include "display.h"
#include <Wire.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 init failed!");
        while (1);
    }
    display.clearDisplay();
    display.setRotation(1);
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(18, 16);
    display.print("HELLO");
    display.display();
    delay(1000);
}

void displayBMX(double ave, int MO, float cm, float mass, float magnitude) {
    display.clearDisplay();
    char txt[128];
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("sensor");
    sprintf(txt, "%d.%d  ", (int)ave, (int)(magnitude * 10) % 10);
    display.setCursor(15, 10);
    display.print(txt);

    display.setCursor(0, 22);
    display.print("set");
    display.setCursor(50, 22);
    display.print("cm");
    sprintf(txt, "%d ", MO + 2);
    display.setCursor(30, 22);
    display.print(txt);

    display.setCursor(50, 34);
    display.print("cm");
    sprintf(txt, "%d.%d  ", (int)cm, (int)(cm * 10) % 10);
    display.setCursor(20, 34);
    display.print(txt);

    // 規格・待機状態の表示
    display.setTextSize(3);
    display.setCursor(0, 90);
    if (mass > IDLE_MASS_THRESHOLD || mass <= 0) {
        display.print("IDLE"); // 何も持っていない時
    } else if (mass < 8) {
        display.print(" S ");
    } else if (mass < 10) {
        display.print(" M ");
    } else if (mass < 14) {
        display.print(" L ");
    } else if (mass < 21) {
        display.print(" 2L ");
    } else {
        display.print(" 3L ");
    }

    // 音声のON,OFFを表示
    display.setTextSize(1);
    display.setCursor(0, 120);
    if (mp3Mode == 1) {
        display.print(" Sound ON");
    } else {
        display.print(" Sound OFF");
    }
    display.display();
}