#include "BLEServer.h"
#include "BLEDevice.h"
#include "BLEAddress.h"
#include <M5StampC3LED.h>

#define SENSOR 8
#define CYCLE (5 * 1000000LL)

M5StampC3LED led = M5StampC3LED();
RTC_DATA_ATTR static uint8_t seq;

void setup() {
  Serial.begin(115200);

  pinMode(SENSOR, INPUT_PULLUP);
  int stat = digitalRead(SENSOR);

  BLEDevice::init("tojimari");
  BLEServer *pServer = BLEDevice::createServer();
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x06);

  std::string strServiceData = "";
  strServiceData += (char)0x05;
  strServiceData += (char)0xff;
  strServiceData += (char)0xff;
  strServiceData += (char)0xff;
  strServiceData += (char)seq;
  strServiceData += (char)stat;
  oAdvertisementData.addData(strServiceData);

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAdvertisementData(oAdvertisementData);

  stat == 0 ? led.show(0, 8, 0) : led.show(8, 0, 0);
  pAdvertising->start();
  delay(1 * 1000);
  pAdvertising->stop();
  led.show(0, 0, 0);

  seq++;

  esp_deep_sleep(CYCLE);
}

void loop() {
}
