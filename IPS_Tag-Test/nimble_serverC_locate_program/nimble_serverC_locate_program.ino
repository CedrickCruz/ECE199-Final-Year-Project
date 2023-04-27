#include <NimBLEDevice.h>

// Initialize all pointers
BLEServer* pServer = NULL;                      // Pointer to the server
BLECharacteristic* pCharacteristic = NULL;    // Pointer to Characteristic 2

// Variables to keep track on device connected
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Variable that will continously be increased and written to the client
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "eb70c0a6-e7fb-469a-bdf7-75c6a559d1ed"
#define CHARACTERISTIC_UUID "3e805ac6-cdc5-4b0c-84a6-5896a6c835d1"

#define SERVER_A "EC:62:60:9D:86:7A"
#define SERVER_B "EC:62:60:9C:06:66"
#define SERVER_C "EC:62:60:9D:7E:96"

String loc_message = "";

struct ReferencePoint {
    String loc;
    int minRssiA;
    int maxRssiA;
    int minRssiB;
    int maxRssiB;
    int minRssiC;
    int maxRssiC;
}   referencePoints[] = {
    {"MyTable", -85, -50, -83, -71, -85, -62} //Sample format of fingerprint
    //{},
    //{},
    //{},
    //{}
};

const int NUM_POINTS = sizeof(referencePoints) / sizeof(referencePoints[0]);
bool loc_known = false;

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
    BLEDevice::init("SrvrC"); // Up to 5 chars only

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
    if (deviceConnected) {
        // Always advertise for reconnecting
        pServer->startAdvertising();

        // Read Characteristic value of Server
        std::string rxValue = pCharacteristic->getValue(); //Get RSSI from A, B, and C
        String str_a, str_b, str_c;
        int rx_a, rx_b, rx_c;
        String rx_new = String(rxValue.c_str());

        // Separate csv
        int commaIndex = rx_new.indexOf(',');
        int secondCommaIndex = rx_new.indexOf(',', commaIndex + 1);
        str_a = rx_new.substring(0, commaIndex);
        str_b = rx_new.substring(commaIndex + 1, secondCommaIndex);
        str_c = rx_new.substring(secondCommaIndex);

        // Convert to int
        rx_a = atoi(str_a.c_str());
        rx_b = atoi(str_b.c_str());
        rx_c = atoi(str_c.c_str());

        //compare to database here-------------------------------------------------------------
        int maxRssi = -100;
        int rx_int = atoi(rxValue.c_str());//left off here--------------------------------------------
        if (rx_int == 1) {
            for (int i = 0; i < NUM_POINTS; i++) {
                if (rx_a >= referencePoints[i].minRssiA && rx_a <= referencePoints[i].maxRssiA /*&& rx_a > maxRssi*/) {
                    Serial.print("From Server A: ");
                    Serial.println(referencePoints[i].loc.c_str());
                    loc_message = referencePoints[i].loc.c_str();
                    loc_message += "A, ";
                    loc_known = true;
                }
                if (rx_b >= referencePoints[i].minRssiB && rx_b <= referencePoints[i].maxRssiB /*&& rx_b > maxRssi*/) {
                    Serial.print("From Server B: ");
                    Serial.println(referencePoints[i].loc.c_str());
                    loc_message += referencePoints[i].loc.c_str();
                    loc_message += "B, ";
                    loc_known = true;
                }
                if (rx_c >= referencePoints[i].minRssiC && rx_c <= referencePoints[i].maxRssiC /*&& rx_c > maxRssi*/) {
                    Serial.print("From Server C: ");
                    Serial.println(referencePoints[i].loc.c_str());
                    loc_message += referencePoints[i].loc.c_str();
                    loc_message += "C";
                    loc_known = true;
                }
                if (loc_known) { // maybe set 3 different loc_known from each server and set value after all three are true
                    Serial.println(loc_message);
                    pCharacteristic->setValue(loc_message);
                }
                else if (!loc_known) {
                    Serial.println("Unknown location");
                    loc_message = "";
                    pCharacteristic->setValue("Unknown location");
                }
            }
            delay(1000);
        }
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

bool is_int(int x) {
    return true;
}
