#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>
#include <SPI.h>
#include <Animator.h>

// User input config
#define POTENTIOMETER_PIN A0
#define BUTTON_PIN 2
int prevButtonState = HIGH;

// Led matrix config
#define NUM_ROWS 10
#define NUM_COLUMNS 20
#define NUM_LEDS (NUM_ROWS * NUM_COLUMNS)
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

// set up variables using the SD utility library functions:
File root;
File myFile;
const int chipSelect = 4;

uint16_t fps = 24;

uint16_t XY(uint8_t x, uint8_t y) {
  if (x > NUM_COLUMNS) {
    return NUM_COLUMNS;
  }
  if (y > NUM_ROWS) {
    return NUM_ROWS;
  }

  if (x % 2 == 1) {
    return ((x + 1) * 10) - y - 1;
  }
  return (x * 10) + y;
}

Animator animator;

void setup() {
  // Only in development
  // Initialize serial connection
  Serial.begin(9600);
  Serial.println("Serial connected.");

  // Initialize button to cycle animations
  pinMode(BUTTON_PIN, INPUT);

  // Initialize LED array
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(8);
  Serial.println("Initialized FastLED.");

  // Initialize root folder
  SD.begin(4);
  root = SD.open("ANIM");
  myFile = root.openNextFile();
  if (!myFile) {
      Serial.println("Could not open first file.");
  } else {
    Serial.println((String)"Opened first file: " + myFile.name());
  }

  animator = Animator(XY);
  animator.loadAnimation(myFile);
}

void loop() {
  EVERY_N_MILLISECONDS(1000 / fps) {
    animator.renderFrame(leds, NUM_LEDS, myFile);
    FastLED.show();
  }

  // Check for button press and potentiometer change very 16ms
  // 30fps
  EVERY_N_MILLISECONDS(1000 / 60) {
    int newButtonState = digitalRead(BUTTON_PIN);

    if (newButtonState == LOW) {
      prevButtonState = LOW;
    }

    if (newButtonState == HIGH && prevButtonState == LOW) {
      prevButtonState = HIGH;
      myFile = root.openNextFile();
      if (!myFile) {
        root.rewindDirectory();
        myFile = root.openNextFile();
      Serial.println((String)"Could not open next file, rewind directory.");
      }
      animator.loadAnimation(myFile);
      Serial.println((String)"Opened next file: " + myFile.name());
    }

    int sensorValue = analogRead(POTENTIOMETER_PIN);
    int brightness = map(sensorValue, 0, 1023, 8, 204);
    FastLED.setBrightness(brightness);
  }
}