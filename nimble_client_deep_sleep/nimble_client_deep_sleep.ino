#include <NimBLEDevice.h>
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>
#include <esp_pm.h>

#define SLEEP_TIME_SECONDS 5
#define RSSI_THRESHOLD -100
#define IDLE_TIME_SECONDS 10

// Define UUIDs:
static BLEUUID serviceUUID_A("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID    charUUID_A("4943d35e-09a0-4d49-85a7-2195596285eb");

static BLEUUID serviceUUID_B("07ec0337-0f0b-4ae7-8e85-ddc8281993f0");
static BLEUUID    charUUID_B("a5288c1e-1b31-4a8c-b7bd-d021d9cbdd32");

static BLEUUID serviceUUID_C("eb70c0a6-e7fb-469a-bdf7-75c6a559d1ed");
static BLEUUID    charUUID_C("3e805ac6-cdc5-4b0c-84a6-5896a6c835d1");

// Variables to keep track on device connected
static boolean doConnectA = false;
static boolean connectedA = false;
static boolean doScanA = false;

static boolean doConnectB = false;
static boolean connectedB = false;
static boolean doScanB = false;

static boolean doConnectC = false;
static boolean connectedC = false;
static boolean doScanC = false;

// Define pointer for the BLE connection
static BLEAdvertisedDevice* myDeviceA;
static BLEAdvertisedDevice* myDeviceB;
static BLEAdvertisedDevice* myDeviceC;

BLERemoteCharacteristic* pRemoteCharacteristicA;
BLERemoteCharacteristic* pRemoteCharacteristicB;
BLERemoteCharacteristic* pRemoteCharacteristicC;

BLEClient*  pClientA  = BLEDevice::createClient();
BLEClient*  pClientB  = BLEDevice::createClient();
BLEClient*  pClientC  = BLEDevice::createClient();
BLEScan* pBLEScan;

// Variables for certain functions
std::string server;
String str_rssi;
String rssi_to_server;
int scanTime = 5; // Scan interval
int sleep_count = 0;

const int BUZZER_PIN = 18; // buzzer pin number
const int PWM_CHANNEL = 0; // PWM channel

//-------------------------------------ClientCallback class----------------------------------------------
// Callback function that is called whenever a client is connected or disconnected
class MyClientCallbackA : public BLEClientCallbacks {
        void onConnect(BLEClient* pclient) {
        }
        void onDisconnect(BLEClient* pclient) {
            doScanA = false;
            connectedA = false;
            Serial.println("onDisconnect A");
        }
};
class MyClientCallbackB : public BLEClientCallbacks {
        void onConnect(BLEClient* pclient) {
        }
        void onDisconnect(BLEClient* pclient) {
            doScanB = false;
            connectedB = false;
            Serial.println("onDisconnect B");
        }
};
class MyClientCallbackC : public BLEClientCallbacks {
        void onConnect(BLEClient* pclient) {
        }
        void onDisconnect(BLEClient* pclient) {
            doScanC = false;
            connectedC = false;
            Serial.println("onDisconnect C");
        }
};
//-------------------------------------AdvertisedDeviceCallback class----------------------------------------------
// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
        // Called for each advertising BLE server.
        void onResult(BLEAdvertisedDevice* advertisedDevice) {

            // We have found a device, let us now see if it contains the service we are looking for.
            if (advertisedDevice->isAdvertisingService(serviceUUID_A)) {
                myDeviceA = advertisedDevice;
                doConnectA = true;
                doScanA = true;
            } // Found our server A
            
            if (advertisedDevice->isAdvertisingService(serviceUUID_B)) {
                myDeviceB = advertisedDevice;
                doConnectB = true;
                doScanB = true;
            } // Found our server B
            
            if (advertisedDevice->isAdvertisingService(serviceUUID_C)) {
                myDeviceC = advertisedDevice;
                doConnectC = true;
                doScanC = true;
            } // Found our server C
        } // onResult
}; // AdvertisedDeviceCallbacks
//-------------------------------------notify callback-------------------------------------------------
void notifyCB(BLERemoteCharacteristic* pRemoteChar, uint8_t* pData, size_t length, bool isNotify){
    Serial.print("Notify callback: ");
    for (int i = 0; i < length; i++){
        Serial.print(pData[i]);
        Serial.print("-");
    }
    Serial.println();
}
static MyClientCallbackA clientCBA;
static MyClientCallbackB clientCBB;
static MyClientCallbackC clientCBC;
//-------------------------------------connectToServer function----------------------------------------------
// Function that is run whenever a server is connected
bool connectToServerA(BLEUUID serviceUUID, BLEUUID charUUID) {
    Serial.print("Forming a connection to ");
    Serial.println(myDeviceA->getAddress().toString().c_str());
    Serial.println(" - Created client for A");

    //pClientA->setClientCallbacks(new MyClientCallbackA());
    pClientA->setClientCallbacks(&clientCBA, false);
    //delay(1000); // test to avoid packet mismatch------------------------------------------------------

    // Connect to the BLE Server.
    pClientA->connect(myDeviceA);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server A");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteServiceA = pClientA->getService(serviceUUID);
    if (pRemoteServiceA == nullptr) {
        Serial.print(" - Failed to find service ");
        Serial.println(serviceUUID.toString().c_str());
        pClientA->disconnect();
        return false;
    }
    Serial.println(" - Found service of server A");
    
    pRemoteCharacteristicA = pRemoteServiceA->getCharacteristic(charUUID);
    if (pRemoteCharacteristicA == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClientA->disconnect();
        return false;
    }

    Serial.println(" - Found our characteristic A");
    connectedA = true;
    return true;
}

// Function that is run whenever the server B is connected
bool connectToServerB(BLEUUID serviceUUID, BLEUUID charUUID) {
    Serial.print("Forming a connection to ");
    Serial.println(myDeviceB->getAddress().toString().c_str());
    Serial.println(" - Created client for B");

    //pClientB->setClientCallbacks(new MyClientCallbackB());
    pClientB->setClientCallbacks(&clientCBB, false);
    //delay(1000); // test to avoid packet mismatch------------------------------------------------------

    // Connect to the BLE Server.
    pClientB->connect(myDeviceB);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server B");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteServiceB = pClientB->getService(serviceUUID);
    if (pRemoteServiceB == nullptr) {
        Serial.print(" - Failed to find service ");
        Serial.println(serviceUUID.toString().c_str());
        pClientB->disconnect();
        return false;
    }
    Serial.println(" - Found service of server B");
    
    pRemoteCharacteristicB = pRemoteServiceB->getCharacteristic(charUUID);
    if (pRemoteCharacteristicB == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClientB->disconnect();
        return false;
    }

    Serial.println(" - Found our characteristic B");
    connectedB = true;
    return true;
}

// Function that is run whenever the server C is connected
bool connectToServerC(BLEUUID serviceUUID, BLEUUID charUUID) {
    Serial.print("Forming a connection to ");
    Serial.println(myDeviceC->getAddress().toString().c_str());
    Serial.println(" - Created client for C");

    //pClientC->setClientCallbacks(new MyClientCallbackC());
    pClientC->setClientCallbacks(&clientCBC, false);
    //delay(1000); // test to avoid packet mismatch------------------------------------------------------

    // Connect to the BLE Server.
    pClientC->connect(myDeviceC);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server C");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteServiceC = pClientC->getService(serviceUUID);
    if (pRemoteServiceC == nullptr) {
        Serial.print(" - Failed to find service ");
        Serial.println(serviceUUID.toString().c_str());
        pClientC->disconnect();
        return false;
    }
    Serial.println(" - Found service of server C");
    
    pRemoteCharacteristicC = pRemoteServiceC->getCharacteristic(charUUID);
    if (pRemoteCharacteristicC == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClientC->disconnect();
        return false;
    }

    Serial.println(" - Found our characteristic C");
    connectedC = true;
    return true;
}
//-------------------------------------deep sleep function---------------------------------------------
void deep_sleep(){
    pClientA->disconnect();
    pClientB->disconnect();
    pClientC->disconnect();

    Serial.println("Entering deep sleep...");
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_SECONDS * 1000000);
    esp_deep_sleep_start();
}
//-------------------------------------collectRSSI function---------------------------------------------
int rssiA;
int rssiB;
int rssiC;
int rssi_average_A[10];
int rssi_average_B[10];
int rssi_average_C[10];
void collectRSSI(){
    for (int i = 0; i < 100; i++){
        // Collect 10 rssi measurements
        rssiA = pClientA->getRssi();
        while(rssiA <= RSSI_THRESHOLD) rssiA = pClientA->getRssi(); // This line ensures valid rssi reading
        rssi_average_A[i] = rssiA;
        
        rssiB = pClientB->getRssi();
        while(rssiB <= RSSI_THRESHOLD) rssiB = pClientB->getRssi();
        rssi_average_B[i] = rssiB;
        
        rssiC = pClientC->getRssi();
        while(rssiC <= RSSI_THRESHOLD) rssiC = pClientC->getRssi();
        rssi_average_C[i] = rssiC;
    }
}
//-------------------------------------averageRSSI function---------------------------------------------
int average_a = 0;
int average_b = 0;
int average_c = 0;
void averageRSSI(){
    average_a = 0;
    average_b = 0;
    average_c = 0;
    for (int i = 0; i < 10; i++){   // This only adds all RSSI values
        average_a += rssi_average_A[i];
        average_b += rssi_average_B[i];
        average_c += rssi_average_C[i];
    }
    for (int i = 0; i < 10; i++){
        Serial.print(rssi_average_A[i]);
        Serial.print(rssi_average_B[i]);
        Serial.println(rssi_average_C[i]);
    }
}
//-------------------------------------
int sort_desc(const void *cmp1, const void *cmp2){
    int a = *((int *)cmp1);
    int b = *((int *)cmp2);

    return a > b ? -1 : (a < b ? 1 : 0);
}
//-------------------------------------
int modeRSSI(int rssi_array[]){
    qsort(rssi_array, 10, sizeof(rssi_array[0]), sort_desc);
    int modeValue = 0;
    int modeCount = 0;
    int tempCount = 0;
    int tempValue = 0;
    for (int i = 0; i < 10; i++){
        if (rssi_array[i] != tempValue){
            tempValue = rssi_array[i];
            tempCount = 0;
        }
        tempCount++;
        if (tempCount > modeCount){
            modeCount = tempCount;
            modeValue = tempValue;
        }
    }
    return modeValue;
}
//-------------------------------------setup-----------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(1000);

    // Set CPU frequency to 80 MHz NOTE: need to check for confirmation
    esp_pm_config_esp32_t pm_config;
    pm_config.max_freq_mhz = 80;
    esp_pm_configure(&pm_config);
    
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("Tag_A");
    BLEDevice::setMTU(40); // Increase MTU size (default: 20) NOTE: Not working

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(45);  //millisec (how often) tested: 1349, 45
    pBLEScan->setWindow(15);     //millisec (how long) tested: 449, 15
    pBLEScan->setActiveScan(true);
    pBLEScan->start(scanTime, false); //sec

    ledcSetup(PWM_CHANNEL, 2000, 8);
} // End of setup.

//-------------------------------------loop------------------------------------------------------------
void loop() {
    delay(1000);
    memset(rssi_average_A, 0, sizeof(rssi_average_A));
    memset(rssi_average_B, 0, sizeof(rssi_average_B));
    memset(rssi_average_C, 0, sizeof(rssi_average_C));

    // Consider tag as idle by IDLE_TIME_SECONDS to enter deep sleep (20 seconds when still scanning, 10 seconds when all servers are connected)
    // TODO: Need a way to interrupt deep sleep when received a message
    
    //sleep_count++;
    if (sleep_count == IDLE_TIME_SECONDS){
        sleep_count = 0;
        deep_sleep();
    }
    
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    // Connect to servers when found
    if (doConnectA == true) {
        if (connectToServerA(serviceUUID_A, charUUID_A)) {
            Serial.println("We are now connected to a BLE Server A.");
            doConnectA = false;
        } else {
            Serial.println("Failed to connect.");
            deep_sleep();
        }
    }
    if (doConnectB == true) {
        if (connectToServerB(serviceUUID_B, charUUID_B)) {
            Serial.println("We are now connected to a BLE Server B.");
            doConnectB = false;
        } else {
            Serial.println("Failed to connect.");
            deep_sleep();
        }
    }
    if (doConnectC == true) {
        if (connectToServerC(serviceUUID_C, charUUID_C)) {
            Serial.println("We are now connected to a BLE Server C.");
            doConnectC = false;
        } else {
            Serial.println("Failed to connect.");
            deep_sleep();
        }
    }
    
    // If we are connected to a peer BLE Server, wait for command
    if (connectedA) {
        // Read Characteristic value of Server
        std::string rxValue = pRemoteCharacteristicA->readValue();
        Serial.print("From server A: ");
        Serial.println(rxValue.c_str());

        // Read data is "1" corresponds to Scan
        int rx_int = atoi(rxValue.c_str());
        if (rx_int == 1) {
            sleep_count = 0;
            Serial.println("Scanning...");
            collectRSSI();
            //averageRSSI();
            
            //str_rssi = String(average_a/10);  // Convert int to String
            str_rssi = String(modeRSSI(rssi_average_A)); // Get mode of RSSI and convert to String
            rssi_to_server += str_rssi;       // Construct rssi csv
            
            //str_rssi = String(average_b/10);  // Convert int to String
            str_rssi = String(modeRSSI(rssi_average_B)); // Get mode of RSSI and convert to String
            rssi_to_server += "," + str_rssi; // Construct rssi csv
            
            //str_rssi = String(average_c/10);  // Convert int to String
            str_rssi = String(modeRSSI(rssi_average_C)); // Get mode of RSSI and convert to String
            rssi_to_server += "," + str_rssi; // Construct rssi csv
            
            pRemoteCharacteristicA->writeValue(rssi_to_server.c_str(), rssi_to_server.length()); // Send rssi data to Server A
            
            Serial.println("Scan done!");
            Serial.println(rssi_to_server);   // For debugging purposes
            rssi_to_server = "";
            pBLEScan->clearResults(); // Clear memory
            // Enter deep sleep after command
            //deep_sleep();
            
        }
        // Read data "2" corresponds to Ring
        else if (rx_int == 2) {
            sleep_count = 0;
            ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);                                // Attach buzzer pin to pwm channel
            ledcWriteNote(PWM_CHANNEL, NOTE_C, 4);                                 // Configure note to play
            delay(1000);                                                           // Play for 1 sec
            ledcDetachPin(BUZZER_PIN);                                             // Detach pin to stop playing
            pRemoteCharacteristicA->writeValue("Ring done!");                      // Send to prompt to Server when done ringing
            // Enter deep sleep after command
            //deep_sleep();
        }
        // Read data "3" resets tag connection status after disconnecting
        else if (rx_int == 3){
            pRemoteCharacteristicA->writeValue("Waiting for command...");
        }
    }
    if (connectedB) {
        // Read data from Server
        std::string rxValue = pRemoteCharacteristicB->readValue();
        Serial.print("From server B: ");
        Serial.println(rxValue.c_str());

        // Read data is "1" corresponds to Scan
        int rx_int = atoi(rxValue.c_str());
        if (rx_int == 1) {
            sleep_count = 0;
            Serial.println("Scanning...");
            rssiA = pClientA->getRssi();
            str_rssi = String(rssiA);         // Convert int to String
            rssi_to_server += str_rssi;       // Construct rssi csv

            rssiB = pClientB->getRssi();
            str_rssi = String(rssiB);         // Convert int to String
            rssi_to_server += "," + str_rssi; // Construct rssi csv
            
            rssiC = pClientC->getRssi();
            str_rssi = String(rssiC);         // Convert int to String
            rssi_to_server += "," + str_rssi; // Construct rssi csv
            pRemoteCharacteristicB->writeValue(rssi_to_server.c_str(), rssi_to_server.length()); // Send rssi data to Server B
            
            Serial.println("Scan done!");
            Serial.println(rssi_to_server);   // For debugging purposes
            rssi_to_server = "";
            pBLEScan->clearResults(); // Clear memory
            // Enter deep sleep after command
            //deep_sleep();
        }
        // Read data "2" corresponds to Ring
        else if (rx_int == 2) {
            sleep_count = 0;
            ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);                                // Attach buzzer pin to pwm channel
            ledcWriteNote(PWM_CHANNEL, NOTE_C, 4);                                 // Configure note to play
            delay(1000);                                                           // Play for 1 sec
            ledcDetachPin(BUZZER_PIN);                                             // Detach pin to stop playing
            pRemoteCharacteristicB->writeValue("Ring done!");                      // Send to prompt to Server when done ringing
            // Enter deep sleep after command
            //deep_sleep();
        }
        // Read data "3" resets tag connection status after disconnecting
        else if (rx_int == 3){
            pRemoteCharacteristicB->writeValue("Waiting for command...");
        }
    }
    if (connectedC) {
        // Read data from Server
        std::string rxValue = pRemoteCharacteristicC->readValue();
        Serial.print("From server C: ");
        Serial.println(rxValue.c_str());

        // Read data is "1" corresponds to Scan
        int rx_int = atoi(rxValue.c_str());
        if (rx_int == 1) {
            sleep_count = 0;
            Serial.println("Scanning...");
            rssiA = pClientA->getRssi();
            str_rssi = String(rssiA);         // Convert int to String
            rssi_to_server += str_rssi;       // Construct rssi csv

            rssiB = pClientB->getRssi();
            str_rssi = String(rssiB);         // Convert int to String
            rssi_to_server += "," + str_rssi; // Construct rssi csv
            
            rssiC = pClientC->getRssi();
            str_rssi = String(rssiC);         // Convert int to String
            rssi_to_server += "," + str_rssi; // Construct rssi csv
            pRemoteCharacteristicC->writeValue(rssi_to_server.c_str(), rssi_to_server.length()); // Send rssi data to Server C
            
            Serial.println("Scan done!");
            Serial.println(rssi_to_server);   // For debugging purposes
            rssi_to_server = "";
            pBLEScan->clearResults(); // Clear memory
            // Enter deep sleep after command
            //deep_sleep();
        }
        // Read data "2" corresponds to Ring
        else if (rx_int == 2) {
            sleep_count = 0;
            ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);                                // Attach buzzer pin to pwm channel
            ledcWriteNote(PWM_CHANNEL, NOTE_C, 4);                                 // Configure note to play
            delay(1000);                                                           // Play for 1 sec
            ledcDetachPin(BUZZER_PIN);                                             // Detach pin to stop playing
            pRemoteCharacteristicC->writeValue("Ring done!");                      // Send to prompt to Server when done ringing
            // Enter deep sleep after command
            //deep_sleep();
        }
        // Read data "3" resets tag connection status after disconnecting
        else if (rx_int == 3){
            pRemoteCharacteristicC->writeValue("Waiting for command...");
        }
    }
    if (!doScanA || !doScanB || !doScanC){
        NimBLEDevice::getScan()->start(1);
    }
} // End of loop
