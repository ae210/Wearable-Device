#include <Arduino.h>
#include <BLEDevice.h>
#include "config.h"
#include "bleclient.h"
#include <freertos/queue.h>

// ===============================
// 受信パケット定義（送信側と一致）
// 送信側：int32 magneticforce (|B|*100), int32 linacc_milli(|LinAcc|*1000)
// ===============================
struct __attribute__((packed)) MagPkt {
  int32_t magneticforce;
  int32_t linacc_milli;
};

static QueueHandle_t gMagQ = nullptr;

// BLE UUID
BLEUUID serviceUUID("8be436b2-80bf-4cfc-ac62-b5adf8e62b1a");
BLEUUID CHARA_UUID_RX("f0dd7290-2f52-448f-95b9-147f99e90921");

// BLE objects
BLERemoteCharacteristic *pRemoteCharacteristicRX = nullptr;
BLEAdvertisedDevice *targetDevice = nullptr;
static BLEClient *gClient = nullptr;

// 外部参照される状態フラグ
bool doConnect = false;
bool deviceConnected = false;
bool bInAlarm = false;

// センサ値（外部から読む用）
float m = 0.0f;      // 磁気[uT]
float aLin = 0.0f;   // 線形加速度[m/s^2]

// ===============================
// キューで受けた値を安全に反映するタスク
// ===============================
static void magProcTask(void*) {
  MagPkt pkt;
  for (;;) {
    if (xQueueReceive(gMagQ, &pkt, portMAX_DELAY) == pdTRUE) {
      float mag_uT   = pkt.magneticforce / 100.0f;
      float acc_ms2  = pkt.linacc_milli  / 1000.0f;

      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        m = mag_uT;
        aLin = acc_ms2;
        xSemaphoreGive(dataMutex);
      }
    }
  }
}

void bleClientInitOnce() {
  if (!gMagQ) {
    gMagQ = xQueueCreate(32, sizeof(MagPkt));
    xTaskCreate(magProcTask, "magProc", 2048, nullptr, 1, nullptr);
  }
}

// ===============================
// BLE callbacks
// ===============================
class funcClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient *pClient) override {
    (void)pClient;
    // 特に無し
  }
  void onDisconnect(BLEClient *pClient) override {
    (void)pClient;
    deviceConnected = false;
    Serial.println("Disconnected from server.");
  }
};

class advertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.getServiceUUID().equals(serviceUUID)) {

      BLEDevice::getScan()->stop();

      // 古いターゲット削除
      if (targetDevice != nullptr) {
        delete targetDevice;
        targetDevice = nullptr;
      }
      targetDevice = new BLEAdvertisedDevice(advertisedDevice);

      doConnect = true;
      Serial.println("Target BLE device found. Ready to connect.");
    }
  }
};

// ===============================
// Notify callback（ここでは m/aLin を書かない！）
// ===============================
static void notifyCallback(BLERemoteCharacteristic*, uint8_t *pData, size_t length, bool) {
  // 正常パケットは 8バイト想定（MagPkt）
  if (length >= sizeof(MagPkt)) {
    MagPkt pkt;
    memcpy(&pkt, pData, sizeof(MagPkt));
    (void)xQueueSend(gMagQ, &pkt, 0);   // 失敗しても次が来るので無視
  }
}

// ===============================
// BLE scan
// ===============================
void startBLEScan() {
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new advertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);

  // ※ 0 は「止めない」挙動になることがあります（あなたの今の運用に合わせて維持）
  pBLEScan->start(0, nullptr, false);

  Serial.println("BLE scan started.");
}

bool shouldConnect() {
  return doConnect;
}

// ===============================
// connect
// ===============================
bool connectToServer() {
  if (targetDevice == nullptr) {
    Serial.println("No target BLE device.");
    return false;
  }

  // 既存クライアントがあれば掃除
  if (gClient != nullptr) {
    if (gClient->isConnected()) gClient->disconnect();
    delete gClient;
    gClient = nullptr;
  }

  gClient = BLEDevice::createClient();
  gClient->setClientCallbacks(new funcClientCallbacks());

  Serial.println("Attempting BLE connection...");

  if (!gClient->connect(targetDevice)) {
    Serial.println("Failed to connect.");
    delete gClient;
    gClient = nullptr;
    return false;
  }

  BLERemoteService *pRemoteService = gClient->getService(serviceUUID);
  if (!pRemoteService) {
    Serial.println("Service not found.");
    gClient->disconnect();
    delete gClient;
    gClient = nullptr;
    return false;
  }

  pRemoteCharacteristicRX = pRemoteService->getCharacteristic(CHARA_UUID_RX);
  if (!pRemoteCharacteristicRX || !pRemoteCharacteristicRX->canNotify()) {
    Serial.println("RX characteristic not found or cannot notify.");
    gClient->disconnect();
    delete gClient;
    gClient = nullptr;
    return false;
  }

  pRemoteCharacteristicRX->registerForNotify(notifyCallback);
  Serial.println("Notification registration complete.");

  deviceConnected = true;
  doConnect = false;
  return true;
}
bool isConnected() {
    return deviceConnected;
}
