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
const int NUM_LOCATIONS = 5;
const String LOCATION_NAMES[NUM_LOCATIONS] = {
    "Area A",
    "Area B",
    "Area C",
    "Area D",
    "Area E"
    //"Area F"
};
const int RSSI_DB[NUM_LOCATIONS][NUM_BEACONS] = {
    { -58, -70, -81},
    { -67, -77, -84},
    { -75, -61, -77},
    { -79, -73, -79},
    { -87, -81, -68}
    //{ -80, -82, -76}
};

String estimatedLocation = "Unknown";
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
            //Serial.println("Sending notification to clients");
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
            //Serial.println(str);
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
//------------------------------------getAverage function----------------------------------------------
int RSSI_AVERAGE[NUM_LOCATIONS] = {0, 0, 0, 0, 0};
int* difference_ptr;
void getAverage(int listA[], int listB[], int listC[]) {
    memset(RSSI_AVERAGE, 0, sizeof(RSSI_AVERAGE));
    for (int i = 0; i < NUM_LOCATIONS; i++) {
        RSSI_AVERAGE[i] = (listA[i] + listB[i] + listC[i]) / 3;
        Serial.println(RSSI_AVERAGE[i]);
    }
}
//------------------------------------print_locationDiff function--------------------------------------
void print_locationDiff(int list_diff_A[], int list_diff_B[], int list_diff_C[]){
    for (int i = 0; i < NUM_LOCATIONS; i++) {
        Serial.println("---------");          // debugging purposes only
        Serial.print("A: ");
        Serial.println(list_diff_A[i]);
        Serial.print("B: ");
        Serial.println(list_diff_B[i]);
        Serial.print("C: ");
        Serial.println(list_diff_C[i]);
    }
}
//------------------------------------estimate_location function---------------------------------------
String estimate_location(int rssi_average[], const String location_names[]) {
    int minDistance = INT_MAX;
    String location = "Unknown";
    for (int i = 0; i < NUM_LOCATIONS; i++){
        if (rssi_average[i] < minDistance){   // Determine area/s with shortest distance to tag
            minDistance = rssi_average[i];
            location = location_names[i];
        }
        if (rssi_average[i] == minDistance){
            //location += location_names[i];
        }
    }
    return location;
}
//------------------------------------getLocation function---------------------------------------------
void getLocation(int rssi_a, int rssi_b, int rssi_c) {
    int LOCATION_DIFF_A[NUM_LOCATIONS] = {0, 0, 0, 0, 0};
    int LOCATION_DIFF_B[NUM_LOCATIONS] = {0, 0, 0, 0, 0};
    int LOCATION_DIFF_C[NUM_LOCATIONS] = {0, 0, 0, 0, 0};
    for (int i = 0; i < NUM_LOCATIONS; i++) {
        int distanceA = 0;
        int distanceB = 0;
        int distanceC = 0;
        distanceA += abs(rssi_a - RSSI_DB[i][0]);   // Gets absolute difference between current and database RSSI
        distanceB += abs(rssi_b - RSSI_DB[i][1]);   // Lower value means shorter distance
        distanceC += abs(rssi_c - RSSI_DB[i][2]);

        LOCATION_DIFF_A[i] += distanceA;            // Add distance to array for computing average
        LOCATION_DIFF_B[i] += distanceB;
        LOCATION_DIFF_C[i] += distanceC;
    }
    print_locationDiff(LOCATION_DIFF_A, LOCATION_DIFF_B, LOCATION_DIFF_C);
    getAverage(LOCATION_DIFF_A, LOCATION_DIFF_B, LOCATION_DIFF_C);
    estimatedLocation = estimate_location(RSSI_AVERAGE, LOCATION_NAMES);  // Constructs message for the mobile app
    //estimatedLocation = estimatedLocation + "\nA" + RSSI_AVERAGE[0];
    //estimatedLocation = estimatedLocation + "B" + RSSI_AVERAGE[1];
    //estimatedLocation = estimatedLocation + "C" + RSSI_AVERAGE[2];
    //estimatedLocation = estimatedLocation + "D" + RSSI_AVERAGE[3];
    //estimatedLocation = estimatedLocation + "E" + RSSI_AVERAGE[4];
    
    pCharacteristic->setValue(estimatedLocation); // Sets characteristic value to be read by mobile app
}
//------------------------------------setup----------------------------------------------------------
void setup() {
    Serial.begin(115200);

    // Create the BLE Device
    BLEDevice::init("Server_C");
    BLEDevice::setMTU(40); // Increase MTU size (default: 20) NOTE: Not working

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

        // Call getLocation when rx_a value is valid (RSSI values are always < 0)
        if (rx_a < 0) {
            getLocation(rx_a, rx_b, rx_c);
        }
    }
    delay(1000);
    
    // The code below keeps the connection status up-to-date:
    // Disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(1000); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
        pCharacteristic->setValue("3");
        pCharacteristic->notify(true);
    }
    // Connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
