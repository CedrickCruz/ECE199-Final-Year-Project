#include <NimBLEDevice.h>

// Initialize all pointers
BLEServer* pServer = NULL;                    // Pointer to the server
BLECharacteristic* pCharacteristic = NULL;    // Pointer to Characteristic

// Variables to keep track on device connected
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "eb70c0a6-e7fb-469a-bdf7-75c6a559d1ed"
#define CHARACTERISTIC_UUID "3e805ac6-cdc5-4b0c-84a6-5896a6c835d1"

//------------------------------------ServerCallbacks class--------------------------------------------
// Callback function that is called whenever a client is connected or disconnected
class MyServerCallbacks: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) {
            deviceConnected = true;
        };

        void onDisconnect(BLEServer* pServer) {
            deviceConnected = false;
        }
};

//------------------------------------CharacteristicCallbacks class-----------------------------------------
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
        void onRead(NimBLECharacteristic* pCharacteristic) {
            Serial.print("From client (readCB): ");
            Serial.println(pCharacteristic->getValue().c_str());
        };

        void onWrite(NimBLECharacteristic* pCharacteristic) {
            Serial.print("From client (writeCB): ");
            Serial.println(pCharacteristic->getValue().c_str());
        };
        /** Called before notification or indication is sent,
            the value can be changed here before sending if desired.
        */
        void onNotify(NimBLECharacteristic* pCharacteristic) {
            Serial.println("Sending notification to clients");
        };


        /** The status returned in status is defined in NimBLECharacteristic.h.
            The value returned in code is the NimBLE host return code.
        */
        void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
            String str = ("Notification/Indication status code: ");
            str += status;
            str += ", return code: ";
            str += code;
            str += ", ";
            str += NimBLEUtils::returnCodeToString(code);
            Serial.println(str);
        };

        void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
            String str = "Client ID: ";
            str += desc->conn_handle;
            str += " Address: ";
            str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
            if (subValue == 0) {
                str += " Unsubscribed to ";
            } else if (subValue == 1) {
                str += " Subscribed to notfications for ";
            } else if (subValue == 2) {
                str += " Subscribed to indications for ";
            } else if (subValue == 3) {
                str += " Subscribed to notifications and indications for ";
            }
            str += std::string(pCharacteristic->getUUID()).c_str();

            Serial.println(str);
        };
};

static CharacteristicCallbacks chrCallbacks;

//------------------------------------setup----------------------------------------------------------
void setup() {
    Serial.begin(115200);

    // Create the BLE Device
    BLEDevice::init("Server_C"); // Up to 5 chars only UPDATE: nimBLE fixed the char limit

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
    pCharacteristic->setValue("Waiting for command...");
    pCharacteristic->setCallbacks(&chrCallbacks);

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
//------------------------------------loop------------------------------------------------------------
void loop() {
    // notify changed value
    if (deviceConnected) {
        pServer->startAdvertising();

        std::string rxValue = pCharacteristic->getValue();
        pCharacteristic->notify(true);

        //Serial.print("From client: ");
        //Serial.println(rxValue.c_str());

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
