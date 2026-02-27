#ifndef __BLECLIENT_H
#define __BLECLIENT_H

#include <BLEDevice.h>
#include <freertos/semphr.h>

extern float aLin; 
extern float m; // bleclient.cppのグローバル変数 m を参照

extern SemaphoreHandle_t dataMutex;

void startBLEScan(); 
void bleClientInitOnce();
bool shouldConnect();
bool connectToServer();
bool isConnected();

#endif // BLECLIENT_H