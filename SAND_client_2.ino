/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

/*
#include "FastIMU.h"
// #include <Wire.h>

#define IMU_ADDRESS 0x68
#define PERFORM_CALIBRATION
BMI055 IMU;
*/

// The remote service we wish to connect to.
static BLEUUID serviceUUID("A07498CA-AD5B-474E-940D-16F1FBE7E8CD");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("51FF12BB-3ED8-46E5-B4F9-D64E2FEC021B");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static BLEClient* pClient;
boolean newVal = false;
int count = 0;

// ESP32-DevKitM-1 numbering: start top left, counterclockwise, skip last GND
// To convert to 1u, same numbers?
int white = 4; // Pin ?
int red = 5; // Pin ?
int green = 6; // Pin ?
int lastcolor = 0;

/*
calData calib = { 0 };
AccelData accelData;
GyroData gyroData;

boolean EPMS[18]; // Should be all false by default, unchecked
*/

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    /*
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
    */
    newVal = true;
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Disconnected");
    // Another place to put active scan for reconnect?
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    // BLEClient*  pClient  = BLEDevice::createClient();
    pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    // Serial.println("Passed the hanging line");
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // This block can be trimmed
    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);
      

    connected = true;

    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

// Perform continuous scan until desired device discovered
void scan() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, true); // Hopefully, should stop when device found
}

/*
// Given an EPM id, turn it on or off as specified
void onoff(int id, boolean turnOn) {
  // This section now depends on DMUX implementation
  switch(id) {
    // To turn on:
    // If currently magnetic, do nothing
    // If not magnetic, send pulse
    // To turn off:
    // If currently magnetic, send pulse
    // If not magnetic, do nothing
    // This is XOR between EPMS[id] and turnOn
    
    case 0: // There will be 18 cases, one for each EPM
      // For each case, figure out where EPM is wired
      // Then, flip the appropriate pins
      if(turnOn ^ EPM[id]) { // If flipping EPM
        // digitalWrite(pin, HIGH) <- add more/modify as needed 
      } // Else, we don't want to flip
      break;
    case 1:

      break;
  }
}


// Need to number each EPM based on face relative to start
void doMove(int order) {
  // gyroData.gyroX or accelData.accelY are examples of how to get IMU data
  switch(order) {
    case 0: // Move N
      
      break;
    case 1: // Move E
      
      break;
    case 2: // Move S
      
      break;
    case 3: // Move W
      
      break;
    case 4: // Latch N
      
      break;
    case 5: // Latch E
      
      break;
    case 6: // Latch S
      
      break;
    case 7: // Latch W
      
      break;
    default: // Latch Down <- Shouldn't happen given current data types
      
      break;
  }
}
*/

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  /*
  Serial.println("Starting IMU Processes");
  int err = IMU.init(calib, IMU_ADDRESS);
  if(err != 0) {
    Serial.print("Error initializing IMU: ");
    Serial.println(err);
  }

  #ifdef PERFORM_CALIBRATION
    Serial.println("Performing IMU Calibration");
    delay(5000); // Necessary?
    Serial.println("Keep IMU level");
    delay(5000);
    IMU.calibrateAccelGyro(&calib);
    Serial.println("Calibration done!");
    delay(5000); // Necessary?
    IMU.init(calib, IMU_ADDRESS);
  #endif
  //Example includes stuff about setting range? Unsure what it all means
  */

  pinMode(white, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);

  // TURN ON MAIN DATA LINES FOR DMUXES?
} // End of setup.

// This is the Arduino main loop function.
void loop() {
  // UNCOMMENT COUNT LINES
  if(count == 9) {  
    count = 0;
    if(!pClient->isConnected())
      connected = false;
  }
  
  // If not connected, scan until device found, then proceed with loop
  if(!connected) {
    scan();
    doConnect = true;
  }
  

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothing more we will do.");
    }
    doConnect = false;
  }

  if(newVal && connected) {
    /* To convert to use in project, parse readValue differently
     *  and use in desired application
     */
    /* Instead of readValue, could do readUInt8, but that only reads one byte */
    std::string str = pRemoteCharacteristic->readValue();
    int len = str.size();
    char *c = new char[len + 1];
    std::copy(str.begin(), str.end(), c);
    c[len] = '\0';
    /* c is now a string with the contents of the characteristic */
    Serial.println(c);

    // Demonstration code 
    if(strcmp(c, "white") == 0) {
      if(lastcolor) {
        digitalWrite(lastcolor, LOW);
      }
      digitalWrite(white, HIGH);
      lastcolor = white;
    } else if(strcmp(c, "green") == 0) {
      if(lastcolor) {
        digitalWrite(lastcolor, LOW);
      }
      digitalWrite(green, HIGH);
      lastcolor = green;
    } else if(strcmp(c, "red") == 0) {
      if(lastcolor) {
        digitalWrite(lastcolor, LOW);
      }
      digitalWrite(red, HIGH);
      lastcolor = red;
    } else { // Turn off the 
      digitalWrite(lastcolor, LOW);
      lastcolor = 0;
    }
    
    newVal = false;

    /* // have id defined locally, unique for each bot before flashing
     * order = str.at(id); // Modify when increasing number of possible orders
     * // ^^ note that we use atoi below when changing possible number of orders
     * doMove(atoi(order));
     */
    
  }
  
  delay(1000); // Delay a second between loops.
  count = count + 1;
} // End of loop
