#include <NimBLEDevice.h>
#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>

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

int scanTime = 5; // Scan interval
BLEScan* pBLEScan;
std::string server;
String str_rssi;
String rssi_to_server;
int rssiA;
int rssiB;
int rssiC;
//String server_name;

const int BUZZER_PIN = 18; // buzzer pin number
const int PWM_CHANNEL = 0; // PWM channel

//-------------------------------------ClientCallback class----------------------------------------------
// Callback function that is called whenever a client is connected or disconnected
class MyClientCallbackA : public BLEClientCallbacks {
        void onConnect(BLEClient* pclient) {
        }

        void onDisconnect(BLEClient* pclient) {
            connectedA = false;
            Serial.println("onDisconnect A");
        }
};
class MyClientCallbackB : public BLEClientCallbacks {
        void onConnect(BLEClient* pclient) {
        }

        void onDisconnect(BLEClient* pclient) {
            connectedB = false;
            Serial.println("onDisconnect B");
        }
};
class MyClientCallbackC : public BLEClientCallbacks {
        void onConnect(BLEClient* pclient) {
        }

        void onDisconnect(BLEClient* pclient) {
            connectedC = false;
            Serial.println("onDisconnect C");
        }
};
//-------------------------------------connectToServer function----------------------------------------------
// Function that is run whenever a server is connected
bool connectToServerA(BLEUUID serviceUUID, BLEUUID charUUID) {
    Serial.print("Forming a connection to ");
    Serial.println(myDeviceA->getAddress().toString().c_str());
    Serial.println(" - Created client for A");

    pClientA->setClientCallbacks(new MyClientCallbackA());

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

    pClientB->setClientCallbacks(new MyClientCallbackB());

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

    pClientC->setClientCallbacks(new MyClientCallbackC());

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
//-------------------------------------setup----------------------------------------------
void setup() {
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("");

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349); //millisec
    pBLEScan->setWindow(449); //millisec
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false); //sec

    ledcSetup(PWM_CHANNEL, 2000, 8);
} // End of setup.

//-------------------------------------loop----------------------------------------------
void loop() {

    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnectA == true) {
        if (connectToServerA(serviceUUID_A, charUUID_A)) {
            Serial.println("We are now connected to a BLE Server A.");
        } else {
            Serial.println("We have failed to connect to the server A; there is nothin more we will do.");
        }
        delay(1000); // Added delay to avoid packet count mismatch
        doConnectA = false;
    }
    if (doConnectB == true) {
        if (connectToServerB(serviceUUID_B, charUUID_B)) {
            Serial.println("We are now connected to a BLE Server B.");
        } else {
            Serial.println("We have failed to connect to the server B; there is nothin more we will do.");
        }
        delay(1000); // Added delay to avoid packet count mismatch
        doConnectB = false;
    }
    if (doConnectC == true) {
        if (connectToServerC(serviceUUID_C, charUUID_C)) {
            Serial.println("We are now connected to a BLE Server C.");
        } else {
            Serial.println("We have failed to connect to the server C; there is nothin more we will do.");
        }
        delay(1000); // Added delay to avoid packet count mismatch
        doConnectC = false;
    }

    //if (connectedA && connectedB && connectedC) BLEDevice::getScan()->stop();
    //else BLEDevice::getScan()->start(5, false);

    // If we are connected to a peer BLE Server, wait for command
    if (connectedA) {
        // Read Characteristic value of Server
        std::string rxValue = pRemoteCharacteristicA->readValue();
        Serial.print("From server A: ");
        Serial.println(rxValue.c_str());

        // Read data is "1" corresponds to Scan
        int rx_int = atoi(rxValue.c_str());
        if (rx_int == 1) {
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
            pRemoteCharacteristicA->writeValue(rssi_to_server.c_str(), rssi_to_server.length()); // Send rssi data to Server A
            
            Serial.println("Scan done!");
            rssi_to_server = "";
            pBLEScan->clearResults(); // Clear memory
            //rx_int = 0; // Can change to location when database is complete?
        }
        // Read data "2" corresponds to Ring
        else if (rx_int == 2) {
            ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);                                // Attach buzzer pin to pwm channel
            ledcWriteNote(PWM_CHANNEL, NOTE_C, 4);                                 // Configure note to play
            delay(1000);                                                           // Play for 1 sec
            ledcDetachPin(BUZZER_PIN);                                             // Detach pin to stop playing
            String txValue = "Ring done!";
            pRemoteCharacteristicA->writeValue(txValue.c_str(), txValue.length()); // Send to prompt to Server when done ringing
            //rx_int = 0;
        }
    }
    else if (doScanA) {
        BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }
    if (connectedB) {
        // Read data from Server
        std::string rxValue = pRemoteCharacteristicB->readValue();
        Serial.print("From server B: ");
        Serial.println(rxValue.c_str());

        // Read data is "1" corresponds to Scan
        int rx_int = atoi(rxValue.c_str());
        if (rx_int == 1) {
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
            rssi_to_server = "";
            pBLEScan->clearResults(); // Clear memory
            //rx_int = 0; // Can change to location when database is complete?
        }
        // Read data "2" corresponds to Ring
        else if (rx_int == 2) {
            ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);                                // Attach buzzer pin to pwm channel
            ledcWriteNote(PWM_CHANNEL, NOTE_C, 4);                                 // Configure note to play
            delay(1000);                                                           // Play for 1 sec
            ledcDetachPin(BUZZER_PIN);                                             // Detach pin to stop playing
            String txValue = "Ring done!";
            pRemoteCharacteristicB->writeValue(txValue.c_str(), txValue.length()); // Send to prompt to Server when done ringing
            //rx_int = 0;
        }
    }
    else if (doScanB) {
        BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }
    if (connectedC) {
        // Read data from Server
        std::string rxValue = pRemoteCharacteristicC->readValue();
        Serial.print("From server C: ");
        Serial.println(rxValue.c_str());

        // Read data is "1" corresponds to Scan
        int rx_int = atoi(rxValue.c_str());
        if (rx_int == 1) {
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

            /*
            BLEDevice::getScan()->start(scanTime, false); // Scanning
            //BLEScanResults foundDevices = pBLEScan->start(scanTime, false); // Scanning
            str_rssi = String(rssiA); // Convert int to String
            rssi_to_server = String(server.c_str()); // Convert Server name to String
            rssi_to_server = rssi_to_server + " - RSSI (from loop): " + str_rssi; // Construct message Server name + RSSI
            pRemoteCharacteristicC->writeValue(rssi_to_server.c_str(), rssi_to_server.length()); // Send rssi data to Server
            */
            
            Serial.println("Scan done!");
            rssi_to_server = "";
            pBLEScan->clearResults(); // Clear memory
            //rx_int = 0; // Can change to location when database is complete?
        }
        // Read data "2" corresponds to Ring
        else if (rx_int == 2) {
            ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);                                // Attach buzzer pin to pwm channel
            ledcWriteNote(PWM_CHANNEL, NOTE_C, 4);                                 // Configure note to play
            delay(1000);                                                           // Play for 1 sec
            ledcDetachPin(BUZZER_PIN);                                             // Detach pin to stop playing
            String txValue = "Ring done!";
            pRemoteCharacteristicC->writeValue(txValue.c_str(), txValue.length()); // Send to prompt to Server when done ringing
            //rx_int = 0;
        }
    }
    else if (doScanC) {
        BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }

    delay(1000); // Delay a second between loops.
} // End of loop
