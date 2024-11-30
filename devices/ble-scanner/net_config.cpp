#include <EEPROM.h>
#include "net_config.h"

#define WM_DEBUG_LEVEL 4
#define SEND_INTERVAL_MS 5000

class IPAddressParameter : public WiFiManagerParameter {
public:
  IPAddressParameter(const char *id, const char *placeholder, IPAddress address)
    : WiFiManagerParameter("") {
    init(id, placeholder, address.toString().c_str(), 16, "", WFM_LABEL_BEFORE);
  }

  bool getValue(IPAddress &ip) {
    return ip.fromString(WiFiManagerParameter::getValue());
  }
};

class IntParameter : public WiFiManagerParameter {
public:
  IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
    : WiFiManagerParameter("") {
    init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
  }

  long getValue() {
    return String(WiFiManagerParameter::getValue()).toInt();
  }
};

typedef struct Backup {
  char privateKey[64];
  char publicKey[64];
  char endpointAddress[32];
  uint32_t localIp;
  uint16_t endpointPort;
} Backup;

static WireGuard wg;
static Params params;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void backupParams(Params* params)
{
  Backup backup;
  memset(&backup, 0, sizeof(backup));

  strncpy(backup.privateKey, params->privateKey->getValue(), 64);
  strncpy(backup.publicKey, params->publicKey->getValue(), 64);
  strncpy(backup.endpointAddress, params->endpointAddress->getValue(), 32);
  backup.privateKey[63]='\0';
  backup.publicKey[63]='\0';
  backup.endpointAddress[31]='\0';
  backup.endpointPort = params->endpointPort->getValue();
  if (params->localIp->getValue(*params->ip)) {
    backup.localIp = *params->ip;
    Serial.print("Local IP: ");
    Serial.println(*params->ip); 
  } else {
    Serial.println("Incorrect IP");
  }
  EEPROM.put(0, backup);
  if (EEPROM.commit()) {
      Serial.println("Settings saved");
  } else {
      Serial.println("EEPROM error");
  }
}

Params* restoreParams()
{
  Backup backup;
  memset(&backup, 0, sizeof(backup));

  EEPROM.begin( 512 );
  EEPROM.get(0, backup);
  Serial.println("Settings loaded");

  backup.privateKey[63]='\0';
  backup.publicKey[63]='\0';
  backup.endpointAddress[31]='\0';

  params.ip = new IPAddress(backup.localIp);
  params.localIp = new IPAddressParameter("local_ip", "interface address", *params.ip);
  params.privateKey = new WiFiManagerParameter("private_key", "interface private key", backup.privateKey, 64);
  params.publicKey = new WiFiManagerParameter("public_key", "peer public key", backup.publicKey, 64);
  params.endpointAddress = new WiFiManagerParameter("endpoint_address", "peer endpoint address", backup.endpointAddress, 32);
  params.endpointPort = new IntParameter("endpoint_port", "peer endpoint port", backup.endpointPort, 6);

  return &params;
}

void setupWifi(Params* params)
{
  WiFiManager wifiManager;
  bool isWifiConfigSucceeded = false;
  bool doManualConfig = false;

  wifiManager.addParameter(params->privateKey);
  wifiManager.addParameter(params->localIp);
  wifiManager.addParameter(params->publicKey);
  wifiManager.addParameter(params->endpointAddress);
  wifiManager.addParameter(params->endpointPort);
  wifiManager.setAPCallback(configModeCallback);
 
  pinMode(3, INPUT_PULLUP);
  int stat = digitalRead(3);
  if (stat == 0) {
      doManualConfig = true;
  }
  if (doManualConfig) {
    Serial.println("wifiManager.startConfigPortal()");
    if (wifiManager.startConfigPortal()) {
      isWifiConfigSucceeded = true;
      Serial.println("startConfigPortal() connect success!");
    }
    else {
      Serial.println("startConfigPortal() connect failed!");
    }
  } else {
    Serial.println("wifiManager.autoConnect()");
    if (wifiManager.autoConnect()) {
      isWifiConfigSucceeded = true;
      Serial.println("autoConnect() connect success!");
    }
    else {
      Serial.println("autoConnect() connect failed!");
    }
  }
  if (!isWifiConfigSucceeded)
  {
    for(;;);    
  }
}

void setupArc(Params* params)
{
  configTime(9 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");
  Serial.println(params->privateKey->getValue());
  Serial.println(params->endpointAddress->getValue());
  Serial.println(params->publicKey->getValue());
  Serial.println(params->endpointPort->getValue());
  wg.begin(
    *params->ip,
    params->privateKey->getValue(),
    params->endpointAddress->getValue(),
    params->publicKey->getValue(),
    (uint16_t)params->endpointPort->getValue()
  );
}