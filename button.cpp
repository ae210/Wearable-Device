#include <Arduino.h>
#include "config.h"
#include "button.h"

bool btn1_status = false;
unsigned long lastDebounceTime1 = 0;
bool btn2_status = false;
unsigned long lastDebounceTime2 = 0;
bool btn3_status = false;
unsigned long lastDebounceTime3 = 0;
const unsigned long debounceDelay = 50; 

int mp3Mode = 1; // 初期状態ON
static uint8_t sRotation = 1;

void initButton() {
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);
}

void buttonTask() {
    unsigned long currentTime = millis();

    // ボタン１: キャリブレーション距離変更
    if (digitalRead(BUTTON1_PIN) == BTN_ON && !btn1_status && (currentTime - lastDebounceTime1) > debounceDelay) {
        btn1_status = true;
        lastDebounceTime1 = currentTime;
        calculation();
        MO = (MO + 1) % 5;
    } else if (digitalRead(BUTTON1_PIN) != BTN_ON && btn1_status) {
        btn1_status = false;
        lastDebounceTime1 = currentTime;
    }

    // ボタン2: mp3モード切り替え
    if (digitalRead(BUTTON2_PIN) == BTN_ON && !btn2_status && (currentTime - lastDebounceTime2) > debounceDelay) {
        btn2_status = true;
        lastDebounceTime2 = currentTime;
        mp3Mode = (mp3Mode + 1) % 2;
        Serial.print("mp3Mode = ");
        Serial.println(mp3Mode);
    } else if (digitalRead(BUTTON2_PIN) != BTN_ON && btn2_status) {
        btn2_status = false;
    }

    // ボタン3: 画面回転
    if (digitalRead(BUTTON3_PIN) == BTN_ON && !btn3_status && (currentTime - lastDebounceTime3) > debounceDelay) {
        btn3_status = true;
        lastDebounceTime3 = currentTime;
        sRotation = (sRotation == 1) ? 3 : 1;
        display.setRotation(sRotation);
    } else if (digitalRead(BUTTON3_PIN) != BTN_ON && btn3_status) {
        btn3_status = false;
    }
}

void waitforbutton() {
    while (digitalRead(BUTTON3_PIN) != BTN_ON) delay(10);
    delay(300);
    while (digitalRead(BUTTON3_PIN) == BTN_ON) delay(10);
    delay(300);
}