#include "BLEServer.h"
#include "BLEDevice.h"
#include "BLEAddress.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"
#include "net_config.h"
#include <M5StampC3LED.h>

int scanTime = 5;  //In seconds
BLEScan *pBLEScan;
M5StampC3LED led = M5StampC3LED();

void notify(const char* status, const char* name)
{
  WiFiClient client;
  char json[256] = {0};
  uint8_t buffer[256] = {0};
  size_t bytesToRead = 0;

  sprintf(json, "{\"status\": \"%s\", \"name\": \"%s\"}", status, name);
  Serial.println(json);

  if(!client.connected()) {
    Serial.println("Connecting...");
    if(!client.connect("unified.soracom.io", 23080)) {
      Serial.println("Failed to connect...");
      led.show(8, 0, 0);
      return;
    }
  }

  Serial.println("Sending...");
  client.write(json, strnlen((char*)json, sizeof(json)));
  Serial.println("Sended.");
  led.show(0, 8, 0);

  while((bytesToRead = client.available()) > 0) {
    bytesToRead = bytesToRead > sizeof(buffer) ? sizeof(buffer) : bytesToRead;
    auto bytesRead = client.readBytes(buffer, bytesToRead); 
    Serial.write(buffer, bytesRead);
  }
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == "tojimari") {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      std::string manufactureData = advertisedDevice.getManufacturerData();
      int status = (int)manufactureData[3];
      Serial.printf("Status: %s\n", status == 0 ? "close" : "open");
      notify(status == 0 ? "close" : "open", advertisedDevice.getName().c_str());
    }
  }
};

void setup() {
  Serial.begin(115200);

  Params* params = 0;
  params = restoreParams();
  setupWifi(params);
  setupArc(params);
  backupParams(params);
  delay(1000);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(scanTime * 1000);
  pBLEScan->setWindow(scanTime * 1000);

  led.show(8, 8, 8);
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();
}