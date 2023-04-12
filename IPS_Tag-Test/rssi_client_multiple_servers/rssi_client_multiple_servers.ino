#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>
#include <NimBLEDevice.h>

// Define UUIDs:
static BLEUUID serviceUUID_0("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID    charUUID_0("4943d35e-09a0-4d49-85a7-2195596285eb");

static BLEUUID serviceUUID_1("07ec0337-0f0b-4ae7-8e85-ddc8281993f0");
static BLEUUID    charUUID_1("a5288c1e-1b31-4a8c-b7bd-d021d9cbdd32");

static BLEUUID serviceUUID_2("eb70c0a6-e7fb-469a-bdf7-75c6a559d1ed");
static BLEUUID    charUUID_2("3e805ac6-cdc5-4b0c-84a6-5896a6c835d1");

// Variables to keep track on device connected
static boolean doConnect0 = false;
static boolean doConnect1 = false;
static boolean doConnect2 = false;

static boolean connected0 = false;
static boolean connected1 = false;
static boolean connected2 = false;

static boolean doScan0 = false;
static boolean doScan1 = false;
static boolean doScan2 = false;

static boolean notification0 = false;
static boolean notification1 = false;
static boolean notification2 = false;

// Define pointer for the BLE connection
static BLEAdvertisedDevice* myDevice;
BLERemoteCharacteristic* pRemoteChar_0;

int scanTime = 5; // Scan interval
BLEScan* pBLEScan;
std::string server;
String str_rssi;
String rssi_to_server;
int rssi;

const int BUZZER_PIN = 18; // buzzer pin number
const int PWM_CHANNEL = 0; // PWM channel

// Callback function for Notify function
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData,
                            size_t length,
                            bool isNotify) {
    /*
    if(pBLERemoteCharacteristic->getUUID().toString() == charUUID_1.toString()){

      // convert received bytes to integer
      uint32_t counter = pData[0];
      for (int i = 1; i<length; i++){
        counter = counter | (pData[i] << i*8);
      }

      // print to Serial
      Serial.print("Characteristic 1 (Notify) from server: ");
      Serial.println(counter);
    }
    */
}

// Callback function that is called whenever a client is connected or disconnected
class MyClientCallback0 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected0 = false;
    Serial.println("onDisconnect 0");
  }
};

class MyClientCallback1 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected1 = false;
    Serial.println("onDisconnect 1");
  }
};

class MyClientCallback2 : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected2 = false;
    Serial.println("onDisconnect 2");
  }
};

// Function that is run whenever the server is connected
bool connectToServer() {
    if(connected0){
      return false;
    }
    if(connected1){
      return false;
    }
    if(connected2){
      return false;
    }
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID_0);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID_0.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");
    
    connected0 = true;
    pRemoteChar_0 = pRemoteService->getCharacteristic(charUUID_0);
    if(connectCharacteristic(pRemoteService, pRemoteChar_0) == false)
      connected0 = false;

    if(connected0 ==false){
      pClient->disconnect();
      Serial.println("At least one characteristic UUID not found");
      return false;
    }
    return true;
}

// Function to check Characteristic
bool connectCharacteristic(BLERemoteService *pRemoteService, BLERemoteCharacteristic* l_BLERemoteChar){
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    if (l_BLERemoteChar == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(l_BLERemoteChar->getUUID().toString().c_str());
      return false;
    }
    Serial.println(" - Found our characteristic: " + String(l_BLERemoteChar->getUUID().toString().c_str()));

    if(l_BLERemoteChar->canNotify())
      l_BLERemoteChar->registerForNotify(notifyCallback);

    return true;
}

// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  // Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID_0)) {
      if(connected0){
        //Get RSSI and name of Server
        Serial.print(advertisedDevice.getName().c_str());
        server = advertisedDevice.getName();
        rssi = advertisedDevice.getRSSI();
        Serial.print(" - RSSI: ");
        Serial.print(rssi);
        Serial.println("  ");
        return;
      }
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect0 = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  pBLEScan = BLEDevice::getScan(); // create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  ledcSetup(PWM_CHANNEL, 2000, 8);
} // End of setup.


void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect0 == true) {
    if (connectToServer()) {
      Serial.println("We are now connected0 to the BLE Server 0.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect0 = false;
  }
  if (doConnect1 == true) {
    if (connectToServer()) {
      Serial.println("We are now connected0 to the BLE Server 0.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect1 = false;
  }
  if (doConnect2 == true) {
    if (connectToServer()) {
      Serial.println("We are now connected0 to the BLE Server 0.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect2 = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected0) {
    // Read data from Server
    std::string rxValue = pRemoteChar_0->readValue();
    Serial.print("From server: ");
    Serial.println(rxValue.c_str());

    // If read data is "1", corresponds to Scan, do Scan
    //aFunction(rxValue);
    int rx_int = atoi(rxValue.c_str());
    if(rx_int == 1){
      Serial.println("Scanning...");
      BLEScanResults foundDevices = pBLEScan->start(scanTime, false); // Scanning
      str_rssi = String(rssi); // Convert int to String
      rssi_to_server = String(server.c_str()); // Convert Server name to String
      rssi_to_server = rssi_to_server + " - RSSI: " + rssi; // Construct message Server name + RSSI
      pRemoteChar_0->writeValue(rssi_to_server.c_str(), rssi_to_server.length()); // Send rssi data to Server
      
      Serial.println("Scan done!");
      pBLEScan->clearResults(); // Clear memory
      rxValue = "0"; // Can change to location when database is complete?
    }
    else if (rx_int == 2){
      ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);
      ledcWriteNote(PWM_CHANNEL, NOTE_C, 4);
      delay(1000);
      ledcDetachPin(BUZZER_PIN);
      String txValue = "Ring done!";
      pRemoteChar_0->writeValue(txValue.c_str(),txValue.length());
    }
  }
  else if(doScan0){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  
  delay(1000); // Delay a second between loops.
} // End of loop

void aFunction(int x){
  Serial.println("int");
}

void aFunction(String x){
  Serial.println("String");
}

void aFunction(std::string x){
  Serial.println("std::string");
}

void aFunction(char * x){
  Serial.println("char");
}
