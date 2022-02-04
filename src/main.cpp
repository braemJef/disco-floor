#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>
#include <SPI.h>

// Led matrix config
#define NUM_ROWS 10
#define NUM_COLUMNS 20
#define NUM_LEDS (NUM_ROWS * NUM_COLUMNS)
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

// set up variables using the SD utility library functions:
Sd2Card card;
File myFile;
const int chipSelect = 4;

uint8_t mode;
uint8_t fps;
uint16_t frameAmount;

union ArrayToInteger16 {
  uint8_t array[2];
  uint16_t integer;
};
ArrayToInteger16 converter16;

int XY(int x, int y) {
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

void setup() {
  // Only in development
  // Initialize serial connection
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // Initialize LED array
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(16);

  // Initialize SD card
  if (!SD.begin(4)) {
    Serial.println("Card failed to initialize, or not present");
    return;
  }
  Serial.println("card initialized.");
  
  myFile = SD.open("BRENT002.bin", FILE_READ);
  mode = myFile.read();
  fps = myFile.read();
  converter16.array[0] = myFile.read();
  converter16.array[1] = myFile.read();
  frameAmount = converter16.integer;
  Serial.println((String)"MODE: " + mode);
  Serial.println((String)"FPS: " + fps);
  Serial.println((String)"FRAME_AMOUNT: " + frameAmount);
}

void loop() {
  EVERY_N_MILLISECONDS(1000 / fps) {
    if (myFile.available()) {
      FastLED.clear();
      converter16.array[0] = myFile.read();
      converter16.array[1] = myFile.read();
      uint16_t pixelAmount = converter16.integer;
      converter16.array[0] = myFile.read();
      converter16.array[1] = myFile.read();
      uint16_t repeat = converter16.integer;

      for (uint16_t i = 0; i < pixelAmount; i++) {
        uint8_t x = myFile.read();
        uint8_t y = myFile.read();
        uint8_t r = myFile.read();
        uint8_t g = myFile.read();
        uint8_t b = myFile.read();
        leds[XY(x, y)] = CRGB(r, g, b);
      }
      FastLED.show();
    } else {
        myFile = SD.open("BRENT002.bin", FILE_READ);
        mode = myFile.read();
        fps = myFile.read();
        converter16.array[0] = myFile.read();
        converter16.array[1] = myFile.read();
        frameAmount = converter16.integer;
        Serial.println((String)"MODE: " + mode);
        Serial.println((String)"FPS: " + fps);
        Serial.println((String)"FRAME_AMOUNT: " + frameAmount);
    }
  }
}