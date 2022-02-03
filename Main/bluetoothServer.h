#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define BALISE_ID           103
#define SERVICE_UUID        "150F"
#define CHARACTERISTIC_UUID "151F"

BLEServer *pServer;
BLECharacteristic *baliseIdCharacteristic;
bool deviceConnected = false;
int txValue = 0;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Disconnected");
    pServer->getAdvertising()->start();
  };
};

void bleServerStart() {
  char title[25];
  sprintf(title, "BASTION-BALISE-%d", BALISE_ID);
  BLEDevice::init(title);
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  baliseIdCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  baliseIdCharacteristic->addDescriptor(new BLE2902());
  
  pService->start();
  pServer->getAdvertising()->start();
  Serial.print("Waiting for cellphone to notify balise ");
  Serial.println(BALISE_ID);
  BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void bleServerLoop() {
  if (deviceConnected) {
     baliseIdCharacteristic->setValue(String(BALISE_ID).c_str());
     baliseIdCharacteristic->notify();
  }
}
