#include <NimBLEDevice.h>

// Initialize all pointers
BLEServer* pServer = NULL;                      // Pointer to the server
BLECharacteristic* pCharacteristic = NULL;    // Pointer to Characteristic

// Variables to keep track on device connected
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "eb70c0a6-e7fb-469a-bdf7-75c6a559d1ed"
#define CHARACTERISTIC_UUID "3e805ac6-cdc5-4b0c-84a6-5896a6c835d1"

//-----------------------------------RSSI Databases----------------------------------------------------------
const int NUM_BEACONS = 3;
const int NUM_LOCATIONS = 6;
const String LOCATION_NAMES[NUM_LOCATIONS] = {
    "Area A",
    "Area B",
    "Area C",
    "Area D",
    "Area E",
    "Area F"
};
const int RSSI_DB[NUM_LOCATIONS][NUM_BEACONS] = {
    {-63, -91, -95},
    {-71, -87, -80},
    {-86, -84, -76},
    {-81, -68, -78},
    {-90, -85, -68},
    {-80, -86, -71}
};
int distanceA = 0;
int distanceB = 0;
int distanceC = 0;
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
    BLEDevice::init("Server_C");

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
//------------------------------------loop----------------------------------------------------------
void loop() {
    if (deviceConnected) {
        // Always advertise for reconnecting
        pServer->startAdvertising();
        pCharacteristic->notify(true);

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
        str_c = rx_new.substring(secondCommaIndex + 1);

        // Convert string rssi values to int
        rx_a = atoi(str_a.c_str());
        rx_b = atoi(str_b.c_str());
        rx_c = atoi(str_c.c_str());

        int rssiValues[NUM_BEACONS] = {rx_a, rx_b, rx_c};

        //compare to database here---------------------------------------------------------------------
        //determines distance by difference of rssi values from scan and db
        //the lesser the distance value means the closer the tag to that reference point
        String estimatedLocationA = "Unknown";
        String estimatedLocationB = "Unknown";
        String estimatedLocationC = "Unknown";
        int minDistanceA = INT_MAX;
        int minDistanceB = INT_MAX;
        int minDistanceC = INT_MAX;
        for (int i = 0; i < NUM_LOCATIONS; i++) {
            distanceA += abs(rssiValues[0] - RSSI_DB[i][0]);
            distanceB += abs(rssiValues[1] - RSSI_DB[i][1]);
            distanceC += abs(rssiValues[2] - RSSI_DB[i][2]);

            if (distanceA < minDistanceA){
                minDistanceA = distanceA;
                estimatedLocationA = LOCATION_NAMES[i];
            }
            if (distanceB < minDistanceB){
                minDistanceB = distanceB;
                estimatedLocationB = LOCATION_NAMES[i];
            }
            if (distanceC < minDistanceC){
                minDistanceC = distanceC;
                estimatedLocationC = LOCATION_NAMES[i];
            }

            /*
            for (int j = 0; j < NUM_BEACONS; j++) {
                distance += abs(rssiValues[j] - RSSI_DB[i][j]); //determine value of rssi values compared to rssi db
            }

            if (distance < minDistance) { //set location as estimated location when distance is less
                minDistance = distance;
                estimatedLocation = LOCATION_NAMES[i];
            }
            */
        }

        Serial.print("Estimated location (from A): ");
        Serial.println(estimatedLocationA);
        Serial.print("Estimated location (from B): ");
        Serial.println(estimatedLocationB);
        Serial.print("Estimated location (from C): ");
        Serial.println(estimatedLocationC);

        String estimatedLocation;
        if(estimatedLocationA == estimatedLocationB){
            estimatedLocation = estimatedLocationA;
        }
        if(estimatedLocationB == estimatedLocationC){
            estimatedLocation = estimatedLocationB;
        }
        if(estimatedLocationA == estimatedLocationC){
            estimatedLocation = estimatedLocationC;
        }
        //estimatedLocation = "Estimated location (from A): " + estimatedLocationA + "\nEstimated location (from B): " + estimatedLocationB + "\nEstimated location (from C): " + estimatedLocationC;
        if(rx_a != 0){
            pCharacteristic->setValue(estimatedLocation);
        }

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
