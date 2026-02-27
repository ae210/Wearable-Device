#include <Arduino.h>
#include "config.h"
#include "mp3play.h"
#include <WiFi.h>
#include "SPIFFS.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceSPIFFS *file = nullptr;
AudioOutputI2S *out = nullptr;
AudioFileSourceID3 *id3 = nullptr;

static String currentFile = "";

void stopCurrentPlayback() {
    if (mp3) {
        if (mp3->isRunning()) mp3->stop();
        delete mp3;
        mp3 = nullptr;
    }
    if (id3) { delete id3; id3 = nullptr; }
    if (file) { delete file; file = nullptr; }
    if (out) {
        out->stop();
        delete out;
        out = nullptr;
    }
    currentFile = ""; 
}

void startPlayback(const char *filePath) {
    if (String(filePath) == currentFile && mp3 && mp3->isRunning()) {
        return;
    }
    stopCurrentPlayback();  

    file = new AudioFileSourceSPIFFS(filePath);
    if (!file->isOpen()) {
        Serial.printf("Failed to open file: %s\n", filePath);
        delete file;
        file = nullptr;
        return;
    }

    id3 = new AudioFileSourceID3(file);
    out = new AudioOutputI2S(0, AudioOutputI2S::INTERNAL_DAC);
    if (!out->begin()) {
        Serial.println("Failed to initialize I2S output");
        delete out;
        out = nullptr;
        return;
    }

    mp3 = new AudioGeneratorMP3();
    if (mp3->begin(id3, out)) {
        Serial.printf("Now playing: %s\n", filePath);
        currentFile = String(filePath); 
    } else {
        Serial.println("Failed to begin MP3 decoding");
        stopCurrentPlayback(); 
    }
}

void audioTask(void *parameter) {
    while (true) {
        if (mp3 && mp3->isRunning()) {
            if (!mp3->loop()) {
                Serial.println("Playback finished");
                stopCurrentPlayback();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

void initmp3play() {
    WiFi.mode(WIFI_OFF); 
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
        return;
    }
    xTaskCreatePinnedToCore(audioTask, "Audio Task", 8192, NULL, 3, NULL, 0);
}

// 必要なヘッダや他の関数はそのまま維持してください

void mp3playTask(double ave, float cm, float mass) {
    static double stable_ref = ave;               // 基準となるセンサ値
    static unsigned long stable_timer = millis(); // 静止時間を測るタイマー
    static int lastPlayedRange = -1;              // 前回鳴らしたサイズ (0:待機, 1:S, 2:M...)

    // 1. センサ値 (ave) の変動チェック
    // 現在のセンサ値が、基準値から「10」以上離れたら動いていると判定
    if (abs(ave - stable_ref) > MAG_FLUCTUATION_THRESHOLD) {
        stable_ref = ave;           // 基準値を現在の値に更新
        stable_timer = millis();    // タイマーをリセット
        return;                     // 変動中はここで処理を終了（音は鳴らさない）
    }

    // 2. 「変動が10以内」のまま1秒（1000ms）経過したか？
    if (millis() - stable_timer >= STABLE_TIME_MS) {
        int currentRange = 0; // 0: Idle(待機), 1: S, 2: M, 3: L, 4: 2L, 5: 3L

        // 質量から現在のサイズを判定
        if (mass > IDLE_MASS_THRESHOLD || mass <= 0) { 
            currentRange = 0; // 指を大きく開いている（何も持っていない）
        } else if (mass < 8) {
            currentRange = 1;
        } else if (mass < 10) {
            currentRange = 2;
        } else if (mass < 14) {
            currentRange = 3;
        } else if (mass < 21) {
            currentRange = 4;
        } else {
            currentRange = 5;
        }

        // 3. 音声通知の判定（前回と同じサイズだったら通知しない）
        if (currentRange == 0) {
            // 【重要】指を大きく開いて1秒安定したら「前回のサイズ」をリセットする
            // これをしないと「Mのイチゴ」の次に「別のMのイチゴ」を掴んだ時に音が鳴らなくなります
            lastPlayedRange = 0; 
        } else {
            // イチゴをつまんだ状態で1秒安定し、かつ「前回と違うサイズ」の場合のみ鳴らす
            if (currentRange != lastPlayedRange) {
                lastPlayedRange = currentRange; // 「前回鳴らしたサイズ」を更新
                
                switch (currentRange) {
                    case 1: startPlayback("/s_sound.mp3"); break;
                    case 2: startPlayback("/m_sound.mp3"); break;
                    case 3: startPlayback("/l_sound.mp3"); break;
                    case 4: startPlayback("/2l_sound.mp3"); break;
                    case 5: startPlayback("/3l_sound.mp3"); break;
                }
            }
        }
    }
}