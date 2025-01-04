/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 * 
 * 
 * 
 * This file is modified from the ble_client example available
 * as part of the Arduino ESP32 BLE library; the above text
 * is the original credits
 * This version authored by Eric O'Leary
 */

/**
 * This is a record of personal notes from the research process,
 * focused on the interaction between turning the chip's power on and off
 * and the Bluetooth connection
 * 
 * 
 * Connection notes:
 * If chip already flashed, giving it power, then starting server works
 *    Was there something special about its being flashed beforehand?
 *    If I disconnect power before server, does that allow for this behavior without reflash?
 * Bot connected, server stops: looks like loop fails to check connection
 *  So, restarting server does nothing for connection
 *  Restarting power to bot fails to do anything, requires reflash
 *  Chip fails to recognize it is disconnected
 * If server already started, then chip flashed, connection works
 * 
 * Big discovery:
 *   Telling bot to disconnect from server works
 *   Then, bot's if(!connected) scan(); works, and we reconnect to server
 *   So, pulling plug/stopping server is failing to send disconnect
 *   We can rewrite server so it sends disconnect message along with server exit
 *     Might need a small wait on server disconnect so chip can disconnect before server stops
 *   Next problem, what happens if we unplug/replug?
 *    First, add delay before scanning, so we can disconnect, unpower, repower, and hopefully reconnect
 *    This does not work; repowering during delay makes program freeze
 *  
 * If we unplug then replug, does not necessarily start from the beginning as desired
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

// Setup for matrix math
#include <ArduinoEigenDense.h>
using namespace Eigen;

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

int data1 = 3;
int data2 = 0;
int data3 = 8;
int data4 = 9;
int s16 = 6;
int s17 = 7; 
int inhib = 4;
int i;

/*
// ESP32-DevKitM-1 numbering: start top left, counterclockwise, skip last GND
// To convert to 1u, same numbers?
int white = 4; // Pin ?
int red = 5; // Pin ?
int green = 6; // Pin ?
int lastcolor = 0;
*/

/*
calData calib = { 0 };
AccelData accelData;
GyroData gyroData;

boolean EPMS[18]; // Should be all false by default, unchecked
*/

// Initializing reference faces and rotation matrices
VectorXd paintFace(3);
VectorXd refFace(3);
MatrixXd rotX(3,3);
MatrixXd rotY(3,3);

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

/*
// Using paintFace and refFace, return which position we are in
int config() {
  switch paintFace(0) {
    case 0: // paint is NOT east/west
      switch paintFace(1) {
        case 1: // paint is north
          switch refFace(0) {
            case 1: // ref is east
              break;
            case -1: // ref is west
              break;
            case 0: // ref is up or down
              switch refFace(2) {
                case 1: // ref is up
                  break;
                case -1: // ref is down
                  break;
              }
              break;
          }
          break; // End of paint north
        case -1: // paint is south
          switch refFace(0) {
            case 1: // ref is east
              break;
            case -1: // ref is west
              break;
            case 0: // ref is up or down
              switch refFace(2) {
                case 1: // ref is up
                  break;
                case -1: // ref is down
                  break;
              }
              break;
          }
          break; // End of paint south
        case 0: // paint is up or down
          switch paintFace(2) {
            case 1: // paint is up
              switch refFace(0) {
                case 1: // ref is east
                  break;
                case -1: // ref is west
                  break;
                case 0: // ref is north or south
                  switch refFace(1) {
                    case 1: // ref is north
                      break;
                    case -1: // ref is south
                      break;
                  }
                  break;
              }
              break; // End of paint up
            case -1: // paint is down
              switch refFace(0) {
                case 1: // ref is east
                  break;
                case -1: // ref is west
                  break;
                case 0: // ref is north or south
                  switch refFace(1) {
                    case 1: // ref is north
                      break;
                    case -1: // ref is south
                      break;
                  }
                  break;
              }
              break; // End of paint down
          }
          break; // End of paint up/down
      }
      break; // End of paint NOT east/west
    case 1: // paint is east
      switch refFace(1) {
        case 1: // ref is north
          break;
        case -1: // ref is south
          break;
        case 0: // ref is up or down
          switch refFace(2) {
            case 1: // ref is up
              break;
            case -1: // ref is down
              break;
          }
          break;
      }
      break;
    case -1: // paint is west
      switch refFace(1) {
        case 1: // ref is north
          break;
        case -1: // ref is south
          break;
        case 0: // ref is up or down
          switch refFace(2) {
            case 1: // ref is up
              break;
            case -1: // ref is down
              break;
          }
          break;
      }
      break;
  }
}
* /

// Need to number each EPM based on face relative to start
void doMove(int order) {
  // gyroData.gyroX or accelData.accelY are examples of how to get IMU data
  switch(order) {
    case 0: // Move N
      switch paintFace(0) {
        case 0: // Middle ring
        switch paintFace(1) {
          case 0: 
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

  /*
  pinMode(white, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  */
  // TURN ON MAIN DATA LINES FOR DMUXES?
  pinMode(data1,OUTPUT);
  pinMode(data2,OUTPUT);
  pinMode(data3,OUTPUT);
  pinMode(data4,OUTPUT);
  pinMode(s16,OUTPUT);
  pinMode(s17,OUTPUT);
  pinMode(inhib,OUTPUT);

  /* Paint face is south (-1 in y axis)
   * Ref face is top (1 in z axis)
   * rotX, rotY are 3D 90 degree rotation matrices
   */
  paintFace << 0,-1,0;
  refFace << 0,0,1;
  rotX << 1,0,0,
          0,0,-1,
          0,1,0;
  rotY << 0,0,1,
          0,1,0,
          -1,0,0;
} // End of setup.

// This is the Arduino main loop function.
void loop() {
  // UNCOMMENT COUNT LINES
  if(count == 9) {  
    count = 0;
    Serial.println("Checking connection");
    Serial.println(pClient->isConnected());
    if(!pClient->isConnected()) {
      Serial.println("Not connected");
      connected = false;
    }
  }
  
  // If not connected, scan until device found, then proceed with loop
  if(!connected) {
    Serial.println("5 seconds until scanning");
    delay(5000);
    scan();
    Serial.println("Scan is now complete");
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
    /* Instead of readValue, could do readUInt8, but that only reads one byte */
    // For later use, consider changing to ^^ to just read one bit?
    // If ^^, we have 0 for none, 1-4 for movement, 5 to trigger latch, 6 for disconnect
    std::string str = pRemoteCharacteristic->readValue();
    int len = str.size();
    char *c = new char[len + 1];
    std::copy(str.begin(), str.end(), c);
    c[len] = '\0';
    // c is now a string with the contents of the characteristic
    Serial.println(c);

    if(strcmp(c,"disconnect")==0) {
      pClient->disconnect();
    }

    // c is string of numbers where ith is direction for bot with id i to move

    // TODO: For demonstration code:
    // Hardcode a path with movement in a few directions
    // Latch, disconnect, move one bot, latch a different face (90deg turns and/or 45deg across another)

    /* MAGNET SCHEME:
     * Painted face - faces south (towards viewer)
     * Reference face - top
     * S0 = Bottom
     * S1 = Below painted
     * S2 = Painted
     * S4 = Reference
     * S3,...,S7 continue the middle band
     * S8,...,S11 are rotation away from viewer with S9 as southeast
     * S12 is east
     * S13,...,S15 are part of rotation away from viewer with S14 as southwest
     */

    // Demonstration code for toggling certain magnets
    // Note that S0 is still an active path
    // Could look at system that converts a binary number to which data to write HIGH
    // clkpulse should be changed to be time needed to complete movement
    if(strcmp(c,"S0")==0) {
      digitalWrite(data1,LOW);
      digitalWrite(data2,LOW);
      digitalWrite(data3,LOW);
      digitalWrite(data4,LOW);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S1")==0) {
      digitalWrite(data1,HIGH);
      digitalWrite(data2,LOW);
      digitalWrite(data3,LOW);
      digitalWrite(data4,LOW);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S2")==0) {
      digitalWrite(data1,LOW);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,LOW);
      digitalWrite(data4,LOW);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S3")==0) {
      digitalWrite(data1,HIGH);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,LOW);
      digitalWrite(data4,LOW);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S4")==0) {
      digitalWrite(data1,LOW);
      digitalWrite(data2,LOW);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,LOW);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S5")==0) {
      digitalWrite(data1,HIGH);
      digitalWrite(data2,LOW);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,LOW);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S6")==0) {
      digitalWrite(data1,LOW);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,LOW);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S7")==0) {
      digitalWrite(data1,HIGH);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,LOW);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S8)==0) {
      digitalWrite(data1,LOW);
      digitalWrite(data2,LOW);
      digitalWrite(data3,LOW);
      digitalWrite(data4,HIGH);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S9")==0) {
      digitalWrite(data1,HIGH);
      digitalWrite(data2,LOW);
      digitalWrite(data3,LOW);
      digitalWrite(data4,HIGH);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S10")==0) {
      digitalWrite(data1,LOW);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,LOW);
      digitalWrite(data4,HIGH);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S11")==0) {
      digitalWrite(data1,HIGH);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,LOW);
      digitalWrite(data4,HIGH);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S12")==0) {
      digitalWrite(data1,LOW);
      digitalWrite(data2,LOW);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,HIGH);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S13")==0) {
      digitalWrite(data1,HIGH);
      digitalWrite(data2,LOW);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,HIGH);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S14")==0) {
      digitalWrite(data1,LOW);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,HIGH);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S15")==0) {
      digitalWrite(data1,HIGH);
      digitalWrite(data2,HIGH);
      digitalWrite(data3,HIGH);
      digitalWrite(data4,HIGH);
      delay(2000); //waits 2 secs
    } else if(strcmp(c,"S16")==0) {
      digitalWrite(inhib,HIGH);
      digitalWrite(s16,HIGH);
      delay(2000);
      digitalWrite(s16,LOW);
      digitalWrite(inhib,LOW);
      // Note that inhib toggle will temporarily supress demux, 
      // after these segments it will turn back on to what data1,2,3,4 are still set to
    } else if(strcmp(c,"S17")==0) {
      digitalWrite(inhib,HIGH);
      digitalWrite(s17,HIGH);
      delay(2000);
      digitalWrite(s17,LOW);
      digitalWrite(inhib,LOW);
    }

    // Demonstration code for testing a movement sequence
    if(strcmp(c,"start")==0) {
      
    }
    
    // Get direction for this bot to move, then do the move
    /*
    dir = c[id];
    if(dir == 1) {
      
    } else if(dir == 2) {
      
    } else if(dir == 3) {
      
    } else if(dir == 4) {
      
    } 
    */
    // Idea: Have 5/6 indicate switch to/from latching mode

    /*
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
    */
    
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
