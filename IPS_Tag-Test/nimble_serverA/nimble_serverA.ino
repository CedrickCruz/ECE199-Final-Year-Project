#include <NimBLEDevice.h>

// Initialize all pointers
BLEServer* pServer = NULL;                      // Pointer to the server
BLECharacteristic* pCharacteristic = NULL;    // Pointer to Characteristic 2

// Variables to keep track on device connected
bool deviceConnected = false;
bool oldDeviceConnected = false;

std::string old_message;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "4943d35e-09a0-4d49-85a7-2195596285eb"

// Callback function that is called whenever a client is connected or disconnected
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("SrvrA"); // Up to 5 chars only

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::READ    |
                      NIMBLE_PROPERTY::WRITE   |
                      NIMBLE_PROPERTY::NOTIFY
                    ); 
  pCharacteristic->setValue("Char value here.");

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {
      pServer->startAdvertising();

      std::string rxValue = pCharacteristic->getValue();

      if (rxValue != old_message){
          old_message = rxValue;
          pCharacteristic->notify(true);
      }
      
      Serial.print("From client: ");
      Serial.println(rxValue.c_str());
      
      //delay, can use millis() for production
      delay(1000);
    }
    // The code below keeps the connection status up-to-date:
    // Disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(1000); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // Connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
