#include <WiFi.h>
#include <WiFiManager.h>
#include <WireGuard-ESP32.h>

class IPAddressParameter;
class IntParameter;

typedef struct Params {
  IPAddress* ip;
  IPAddressParameter* localIp;
  WiFiManagerParameter* privateKey;
  WiFiManagerParameter* publicKey;
  WiFiManagerParameter* endpointAddress;
  IntParameter* endpointPort;
} Params;

Params* restoreParams();
void backupParams(Params* params);
void setupWifi(Params* params);
void setupArc(Params* params);