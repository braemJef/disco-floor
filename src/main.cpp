#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>
#include <SPI.h>
#include <AnimatorSimple.h>

// Led matrix config
#define NUM_ROWS 10
#define NUM_COLUMNS 20
#define NUM_LEDS (NUM_ROWS * NUM_COLUMNS)
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

// set up variables using the SD utility library functions:
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

AnimatorSimple simple;

void setup() {
  // Only in development
  // Initialize serial connection
  Serial.begin(9600);
  Serial.println("Serial connected.");

  // Initialize LED array
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(8);
  Serial.println("Initialized FastLED.");

  simple = AnimatorSimple(XY);
  simple.loadAnimation("BREND002.bin");

  Serial.println((String)"MODE: " + simple.mode);
  Serial.println((String)"FPS: " + simple.fps);
  Serial.println((String)"FRAME_AMOUNT: " + simple.frameAmount);
}

void loop() {
  EVERY_N_MILLISECONDS(1000 / fps) {
    simple.renderFrame(leds, NUM_LEDS);
    FastLED.show();
  }
  // EVERY_N_MILLISECONDS(1000 / fps) {
  //   if (myFile.available()) {
  //     FastLED.clear();
  //     converter16.array[0] = myFile.read();
  //     converter16.array[1] = myFile.read();
  //     uint16_t pixelAmount = converter16.integer;
  //     converter16.array[0] = myFile.read();
  //     converter16.array[1] = myFile.read();
  //     uint16_t repeat = converter16.integer;

  //     for (uint16_t i = 0; i < pixelAmount; i++) {
  //       uint8_t x = myFile.read();
  //       uint8_t y = myFile.read();
  //       uint8_t r = myFile.read();
  //       uint8_t g = myFile.read();
  //       uint8_t b = myFile.read();
  //       leds[XY(x, y)] = CRGB(r, g, b);
  //     }
  //     FastLED.show();
  //   } else {
  //       myFile = SD.open("BRENT002.bin", FILE_READ);
  //       mode = myFile.read();
  //       fps = myFile.read();
  //       converter16.array[0] = myFile.read();
  //       converter16.array[1] = myFile.read();
  //       frameAmount = converter16.integer;
  //       Serial.println((String)"MODE: " + mode);
  //       Serial.println((String)"FPS: " + fps);
  //       Serial.println((String)"FRAME_AMOUNT: " + frameAmount);
  //   }
  // }
}