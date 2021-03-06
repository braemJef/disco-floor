#include <Animator.h>

const uint8_t MODE_FADE = 0x00;
const uint8_t MODE_RETAIN = 0x01;
const uint8_t MODE_REPLACE = 0x02;

union ArrayToInteger16 {
  uint8_t bytes[2];
  uint16_t integer;
};

uint8_t getXCoordinate(uint16_t integer) {
  return (uint8_t)((integer & 0xFF00) >> 8);
}

uint8_t getYCoordinate(uint16_t integer) {
  return (uint8_t)(integer & 0x00FF);
}

uint8_t readInt8(File& file) {
  return file.read();
}

uint16_t readInt16(File& file) {
  ArrayToInteger16 converter;
  converter.bytes[0] = file.read();
  converter.bytes[1] = file.read();
  return converter.integer;
}

Animator::Animator() {}
Animator::Animator(uint16_t (*xyFunc)(uint8_t x, uint8_t y)) {
  xy = xyFunc;
}

void Animator::loadAnimation(File& file) {
  file = file;
  loadAnimationInfo(file);
}

void Animator::loadAnimationInfo(File& file) {
  mode = readInt8(file);
  fps = readInt8(file);
  frameAmount = readInt16(file);
  fadePercentage = readInt8(file);

  // Read unused bytes
  for (uint8_t i = 0; i < 11; i++) {
    readInt8(file);
  }

  frame = 0;
}

void Animator::renderFrame(CRGB *leds, uint16_t numLeds, File& file) {
  if (frame == frameAmount - 1) {
    file.seek(0);
    loadAnimationInfo(file);
  }
  framePixelAmount = readInt16(file);
  // Read unused bytes
  for (uint8_t i = 0; i < 6; i++) {
    readInt8(file);
  }

  if (mode == MODE_RETAIN) {
    // Do nothing
  }
  if (mode == MODE_FADE) {
    fadeToBlackBy(leds, numLeds, 25);
  }
  if (mode == MODE_REPLACE) {
    FastLED.clear();
  }

  for (uint16_t i = 0; i < framePixelAmount; i++) {
    uint8_t x = readInt8(file);
    uint8_t y = readInt8(file);
    uint8_t r = readInt8(file);
    uint8_t g = readInt8(file);
    uint8_t b = readInt8(file);
    leds[xy(x, y)] = CRGB(r, g, b);
  }

  frame++;
}

uint8_t Animator::getFps() {
  return fps;
}