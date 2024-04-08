/* Test sketch for Adafruit BNO08x sensor in UART-RVC mode */
#include <Adafruit_BNO08x.h>
#include "Adafruit_BNO08x_RVC.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Adafruit_SSD1306 display(-1);

#define ALPHA 0.98 // Weighting factor for accelerometer data

Adafruit_BNO08x_RVC rvc = Adafruit_BNO08x_RVC();

float prev_r = 0.0;
float prev_i = 0.0;
float prev_j = 0.0;
float prev_k = 0.0;

void eulerToQuaternion(float yaw, float pitch, float roll, float &w, float &x, float &y, float &z) {
  // Convert angles to radians if needed (assuming degrees are provided)
  yaw = radians(yaw);
  pitch = radians(pitch);
  roll = radians(roll);

  // Calculate half angles for better numerical stability
  float cy = cos(yaw * 0.5);
  float sy = sin(yaw * 0.5);
  float cp = cos(pitch * 0.5);
  float sp = sin(pitch * 0.5);
  float cr = cos(roll * 0.5);
  float sr = sin(roll * 0.5);

  // Calculate quaternion components
   
  w = floor((cr * cp * cy + sr * sp * sy)*100+0.5)/100;
  x = floor((sr * cp * cy - cr * sp * sy)*100+0.5)/100;
  y = floor((cr * sp * cy + sr * cp * sy)*100+0.5)/100;
  z = floor((cr * cp * sy - sr * sp * cy)*100+0.5)/100;

  
}

void setup() {
  // ------- Do not modify this section of code ------
  // initialize with the I2C addr 0x3C
  // display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  // // Clear the buffer.
  // display.clearDisplay();
  // --------------------------------------------------
  
  Serial.begin(115200); // This is the baud rate specified by the datasheet
  while (!Serial)
    delay(10);
  

  if (!rvc.begin(&Serial)) { // connect to the sensor over hardware serial
    Serial.println("Could not find BNO08x!");
    while (1)
      delay(10);
  }


  Serial.println("BNO08x found!");
}

void loop() {
  
  BNO08x_RVC_Data heading;
  if (!rvc.read(&heading)) {
    return;
  }
  // Display Text
  // display.clearDisplay();
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  // display.setCursor(0,0);
  // display.print("Yaw: ");
  // display.println(heading.yaw);
  // display.print("Pitch: ");
  // display.println(heading.pitch);
  // display.print("Roll: ");
  // display.print(heading.roll);
  // display.display();
 
  

  // Serial.print(F("Yaw: "));
  // Serial.print(heading.yaw);
  // Serial.print(F("\tPitch: "));
  // Serial.print(heading.pitch);
  // Serial.print(F("\tRoll: "));
  // Serial.println(heading.roll);

  float r = 0.0;
  float i = 0.0;
  float j = 0.0;
  float k = 0.0;

  float ax = heading.x_accel / 100.0;
  float ay = heading.y_accel / 100.0;
  float az = heading.z_accel / 100.0;
  
  // Calculate pitch and roll angles using accelerometer data
  float acc_pitch = atan2(-ax, sqrt(ay * ay + az * az));
  float acc_roll = atan2(ay, sqrt(ax * ax + az * az));
  
  // Apply Complementary Filter to combine accelerometer and gyro data
  float pitch = ALPHA * heading.pitch + (1 - ALPHA) * acc_pitch;
  float roll = ALPHA * heading.roll + (1 - ALPHA) * acc_roll;

  eulerToQuaternion(heading.yaw, heading.pitch, heading.roll, r, i, j, k);

  if (prev_r != r || prev_i != i || prev_j != j || prev_k != k){
    Serial.print("r: ");
    Serial.print(r);
    Serial.print(" i: ");
    Serial.print(j);
    Serial.print(" j: ");
    Serial.print(i);
    Serial.print(" k: ");
    Serial.println(k);
  }

  prev_r = r;
  prev_i = i;
  prev_j = j;
  prev_k = k;


  delay(10);
}