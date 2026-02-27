#ifndef __MP3PLAY_H
#define __MP3PLAY_H

#include <Arduino.h>
#include "config.h"

void initmp3play();
void stopCurrentPlayback();
void startPlayback(const char *filePath);
void audioTask(void *parameter);
// isMovingフラグを追加し、不要な引数を整理
void mp3playTask(double ave, float cm, float mass);

#endif