#ifndef __CONFIG_H
#define __CONFIG_H
#include <Arduino.h>
#include <Wire.h>
#include "display.h"
#include "button.h"
#include "calculation.h"
#include "mp3play.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <freertos/semphr.h> // Mutex用に追加

// 以前のBNO055関連のincludeは削除

extern double ave;
extern float magnitude;
extern float cm;
extern float mass;
extern int mp3Mode, mp3Mode_b;
extern int DO;
extern float pitch;

/* mp3 config */
#define STD_INTERVAL 1500         //mp3再生の閾値 (ms)
#define SOUND_CHECK_INTERVAL 100  //規格変化のチェック周期 (ms)

/* --- 誤検知防止・待機判定用の設定値 --- */
#define MAG_FLUCTUATION_THRESHOLD 10.0 // magnitude(ave)の変動許容範囲
#define STABLE_TIME_MS 1000            // 静止とみなす時間(ms)
#define IDLE_MASS_THRESHOLD 30.0       // 指を大きく開いていると判定する質量(g)

extern SemaphoreHandle_t dataMutex;

#endif