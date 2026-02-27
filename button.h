#ifndef __BUTTON_H
#define __BUTTON_H
#include <Arduino.h>

#define BUTTON1_PIN 2
#define BUTTON2_PIN 15
#define BUTTON3_PIN 13
#define BTN_ON LOW 

void initButton();
void buttonTask();
void waitforbutton();
extern int mp3Mode;
#endif