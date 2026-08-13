/*
  Arduino Nesso N1 - Display Battery Status

  This example will enable battery charging and display its charging state.

  created: December 11 2025
  by: Ubi de Feo

  This example code is in the public domain.
*/
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include <Arduino_Nesso_N1.h>
void renderstatusSprite();

NessoBattery battery;
NessoDisplay display;

const int DISPLAY_WIDTH = 240;
const int DISPLAY_HEIGHT = 135;
const uint16_t COLOR_TEAL = 0x0410;
const uint16_t COLOR_BLACK = 0x0000;
const uint16_t COLOR_GREEN = 0x1e85;
const uint16_t COLOR_ORANGE = 0xed03;
const uint16_t COLOR_RED = 0xe841;
const uint16_t COLOR_BLUE = 0x001F;
const int ANIMATION_DELAY = 30;
const int COLS = 20;
const int ROWS = 1;
const int REGION_WIDTH = 12;
const int REGION_HEIGHT = 135;


LGFX_Sprite statusSprite(&display);

float batteryVoltage = 0.0;
int counter = 0;
char uptimeString[26];
bool ledStatus = false;
unsigned long lastLEDflip = 0;
int progressEdge = 240;
bool progressExpanding = true;

void setup() {
  Serial.begin(115200);

  display.begin();

  battery.begin();
  battery.enableCharge();

  display.setRotation(1);
  display.setEpdMode(epd_mode_t::epd_fastest);

  display.fillScreen(TFT_WHITE);
  display.setTextColor(COLOR_TEAL);
  display.setTextSize(5);
  display.drawString("Nesso N1", 6, 11);

  // statusSprite.createSprite(240, 81);
  statusSprite.createSprite(240, 135);
  delay(1000);
  // pinMode(LED_BUILTIN, OUTPUT);  // All these are defined in Nesso header, do not redefine
  // pinMode(KEY1, INPUT);
  // pinMode(KEY2, INPUT);

  Serial.println("Starting...");
  lastLEDflip = millis();

  Serial.println("Initializing IMU...");
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accel Rate: ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");

  Serial.print("Gyro Rate: ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
}

void loop() {
  unsigned long msNow = millis();
  static uint8_t lastKey1 = false;
  static uint8_t lastKey2 = false;
  static uint8_t lastPwrIn = false;
  float ax, ay, az;
  float gx, gy, gz;

  uint8_t curKey1 = digitalRead(KEY1);
  uint8_t curKey2 = digitalRead(KEY2);
  uint8_t curPwrIn = digitalRead(VIN_DETECT);

  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);

    Serial.print("aX:");
    Serial.print(ax, 2);
    Serial.print(" aY:");
    Serial.print(ay, 2);
    Serial.print(" aZ:");
    Serial.print(az, 2);
    Serial.print("\t gX:");
    Serial.print(gx, 2);
    Serial.print(" gY:");
    Serial.print(gy, 2);
    Serial.print(" gZ:");
    Serial.println(gz, 2);
    if(ay > 0.0) {
      display.setRotation(3);
    } else {
      display.setRotation(1);
    }
}

  if(curKey1 != lastKey1 && !curKey1) {
    Serial.println("KEY1 pressed");
  }
  if(curKey2 != lastKey2 && !curKey2) {
    Serial.println("KEY2 pressed");
  }
  if(curPwrIn != lastPwrIn && !curPwrIn) {
    delay(5000);
    Serial.println("Power in is LOW");
  }
  if(curPwrIn != lastPwrIn && curPwrIn) {
    delay(5000);
    Serial.println("Power in is HIGH (plugged in)");
  }
   
  float chargeLevel = battery.getChargeLevel();
  batteryVoltage = battery.getVoltage();

  char batteryStatusTicker[16];
  sprintf(batteryStatusTicker, "%4.2f %6.2f%%", batteryVoltage, chargeLevel);
  if (msNow - lastLEDflip > 1000) {
    ledStatus = !ledStatus;
    // LED_BUILTIN currently disabled for failures
    digitalWrite(LED_BUILTIN, ledStatus);
    lastLEDflip = msNow;
    Serial.print(batteryVoltage);
    Serial.print(" ");
    Serial.print(chargeLevel);
    Serial.println("%");
    sprintf(uptimeString, "uptime:\n%012d\n", millis() / 1000);
    Serial.println(uptimeString);
  }
  renderstatusSprite();
  lastKey1 = curKey1;
  lastKey2 = curKey2;
  lastPwrIn = curPwrIn;
}

void renderstatusSprite() {
  int offsetY = 18;
  int fullOffsetY = 54;
  statusSprite.fillSprite(TFT_WHITE);
  statusSprite.setTextColor(COLOR_TEAL);
  statusSprite.setTextSize(5);
  statusSprite.drawString("Nesso N1", 6, 11);
  if(!digitalRead(KEY1)) {
    statusSprite.setColor(COLOR_BLUE);
  } else if(!digitalRead(KEY2)) {
    statusSprite.setColor(COLOR_GREEN);
  } else {
    statusSprite.setColor(COLOR_ORANGE);
  }
  if (progressExpanding) {
    statusSprite.fillRect(progressEdge, 0 + fullOffsetY, 240, 8);
  } else {
    statusSprite.fillRect(0, 0 + fullOffsetY, progressEdge, 8);
  }

  progressEdge -= 1;
  if (progressEdge <= 0) {
    progressEdge = 240;
    progressExpanding = !progressExpanding;
  }
  statusSprite.setTextSize(3);
  statusSprite.setTextColor(COLOR_BLACK);
  statusSprite.drawString("Battery:", 6, offsetY + fullOffsetY);
  uint16_t textColor = 0x0000;
  if (batteryVoltage > 3.7) {
    textColor = COLOR_GREEN;
  } else if (batteryVoltage <= 3.7 && batteryVoltage >= 3.3) {
    textColor = COLOR_ORANGE;
  } else {
    textColor = COLOR_RED;
  }
  char batteryString[6];
  sprintf(batteryString, "%4.2f", batteryVoltage);
  statusSprite.setTextColor(textColor);
  statusSprite.drawString(batteryString, 165, offsetY + fullOffsetY);
  statusSprite.setTextColor(COLOR_BLACK);
  statusSprite.setTextSize(2);

  statusSprite.setTextColor(COLOR_TEAL);
  statusSprite.drawString(uptimeString, 6, offsetY + 38 + fullOffsetY);
  // statusSprite.pushSprite(0, 54);
  statusSprite.pushSprite(0, 0);
  //
}
