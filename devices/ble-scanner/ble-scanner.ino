#include "BLEServer.h"
#include "BLEDevice.h"
#include "BLEAddress.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include "BLEAdvertisedDevice.h"
#include "net_config.h"

int scanTime = 5;  //In seconds
BLEScan *pBLEScan;

void notify(int status);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == "tojimari") {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      std::string manufactureData = advertisedDevice.getManufacturerData();
      int status = (int)manufactureData[3];
      Serial.println(status == 0 ? "Close" : "Open");
      notify(status);
    }
  }
};

void notify(int status)
{
  WiFiClient client;
  uint8_t buffer[256] = {0};
  size_t bytesToRead = 0;

  if(!client.connected()) {
    Serial.println("Connecting...");
    if(!client.connect("unified.soracom.io", 23080)) {
      Serial.println("Failed to connect...");
      return;
    }
  }

  Serial.println("Sending...");
  client.write((char*)&status, sizeof(status));
  Serial.println("Sended.");

  while((bytesToRead = client.available()) > 0) {
    bytesToRead = bytesToRead > sizeof(buffer) ? sizeof(buffer) : bytesToRead;
    auto bytesRead = client.readBytes(buffer, bytesToRead); 
    Serial.write(buffer, bytesRead);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  Params* params = 0;
  params = restoreParams();
  setupWifi(params);
  setupArc(params);
  backupParams(params);
  delay(1000);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(1000);
  pBLEScan->setWindow(500);  // less or equal setInterval value
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
  delay(2000);
}