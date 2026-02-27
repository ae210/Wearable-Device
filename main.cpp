#include "config.h"
#include "bleclient.h"

double ave;
int val[10];
double x[CALC_S], y[CALC_S];
double x_val[41];
double adc_val[41];
float mass;
float cm;
float magnitude;
float pitch;

SemaphoreHandle_t dataMutex; // Mutexの実体

// 画面更新用タイマー
unsigned long lastDisplayTime = 0;
const unsigned long DISPLAY_INTERVAL = 150; // 150msごとに画面更新

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    initDisplay(); 
    initButton();  
    initmp3play(); 

    // BLE関連の初期化
    dataMutex = xSemaphoreCreateMutex();
    BLEDevice::init("");
    bleClientInitOnce();
    startBLEScan();

    delay(500);
}

void loop()
{
    unsigned long currentTime = millis();

    // BLEの接続管理
    if (shouldConnect()) {
        connectToServer();
    }

    // BLE側から受信した磁気データを安全に取得
    float current_mag = 0.0;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        current_mag = m; // bleclient.cpp で更新される値
        xSemaphoreGive(dataMutex);
    }
    
    // 取得した値を magnitude に代入して以降の計算へ回す
    magnitude = current_mag;

    delay(50);                                   
    buttonTask();                                 
    
    // 移動平均の計算
    float sum = 0;
    for (int i = 0; i < 9; i++)
    { 
        val[i] = val[i + 1];
        sum += val[i];
    }
    val[9] = magnitude;
    sum += val[9];
    ave = sum / 10.0;

    // キャリブレーションと質量の計算
    for (int i = 0; i < 41; i++) 
    {
        if (ave < adc_val[i])
        {
            cm = x_val[i];
        }
    }
    mass = 5.8136 * cm - 6.1977;
    
    delay(10);

    // 音声再生処理 (main.cppの元コードにあった mode 変数は未定義なので MO に修正しています)
    if (mp3Mode == 1)
    {
        mp3playTask(ave,  cm, mass);
    }
    else if (mp3Mode == 0)
    {
    }

    int mp3Mode_b = mp3Mode;
    if (mp3Mode_b == 0 && mp3Mode == 1)
    {
        Serial.println("Sound ON!!");
    }

    // ディスプレイ表示
    if (currentTime - lastDisplayTime > DISPLAY_INTERVAL)
    {
        displayBMX(ave, MO, cm, mass, magnitude); 
        lastDisplayTime = currentTime;
    }
}