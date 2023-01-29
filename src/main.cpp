#include <Arduino.h>

#include <FastLED.h>
#include <SD.h>
#include <SPI.h>
#include <Animator.h>

// User input config
#define POTENTIOMETER_PIN A0
#define BUTTON_PIN 6
int prevButtonState = HIGH;

// Led matrix config
#define NUM_ROWS 16
#define NUM_COLUMNS 32
#define NUM_LEDS (NUM_ROWS * NUM_COLUMNS)
#define DATA_PIN 0

CRGB leds[NUM_LEDS];

// SD reader config
#define SD_CHIP_SELECT 21

File root;
File myFile;

// Animator config
Animator animator;
uint16_t fps = 24;
unsigned long newMillis = 0;

uint16_t XY(uint8_t x, uint8_t y) {
  if (x > NUM_COLUMNS) {
    return NUM_COLUMNS;
  }
  if (y > NUM_ROWS) {
    return NUM_ROWS;
  }

  if (x % 2 == 1) {
    return ((x + 1) * NUM_ROWS) - y - 1;
  }
  return (x * NUM_ROWS) + y;
}

void setup() {
  Serial.begin(115200); // initialize serial communication at 115200
  delay(5000); // Pause for 2 seconds
  Serial.println("Serial connected.");

  // Initialize button to cycle animations
  pinMode(BUTTON_PIN, INPUT);

  // Initialize FastLED
  FastLED.addLeds<WS2812B, DATA_PIN, EOrder::GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(5);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.clear();
  FastLED.show();
  Serial.println("Initialized FastLED.");
  
  // Initialize root folder
  if (!SD.begin(SD_CHIP_SELECT)) {
    Serial.println("SD initialization failed.");
    while (1);
  }

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
  unsigned long currentMillis = millis();
  if (currentMillis > newMillis) {
    newMillis = currentMillis + (1000 / fps);
    animator.renderFrame(leds, NUM_LEDS, myFile);
    FastLED.show();
  }

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
      fps = animator.fps;
      Serial.println((String)"Opened next file: " + myFile.name());
    }

    // int sensorValue = analogRead(POTENTIOMETER_PIN);
    // int brightness = map(sensorValue, 0, 1023, 8, 204);
    // FastLED.setBrightness(brightness);
  }
}
