#include <Animator.h>
#include <StandardCplusplus.h>
#include <map>

using namespace std;

union ArrayToInteger16 {
  uint8_t bytes[2];
  uint16_t integer;
};

template<size_t N>
Animator<N>::Animator(
      CRGB (&fastLeds)[N],
      uint16_t (*xyFunc)(uint8_t x, uint8_t y),
      File & file
) {
  leds = fastLeds;
  xy = xyFunc;
  data = file;
  loadAnimationInfo();
};

template<size_t N>
void Animator<N>::loadAnimation(File & file) {
  data = file;
  repeat = 0;
  frame = 0;
  loadAnimationInfo();
}

template<size_t N>
void Animator<N>::renderFrame() {
  if (frame == frameAmount) {
    data.seek(0);
    loadAnimationInfo();
  }

  if (repeat == 0) {
    uint16_t pixelAmount = readInt16();
    repeat = readInt16();

    for (uint16_t i = 0; i < pixelAmount; i++) {
      uint8_t x = readInt8();
      uint8_t y = readInt8();
      uint8_t r = readInt8();
      uint8_t g = readInt8();
      uint8_t b = readInt8();
      currentFrame.inser()
      leds[XY(x, y)] = CRGB(r, g, b);
    }
  }

  frame++;
  repeat--;
}

template<size_t N>
void Animator<N>::loadAnimationInfo() {
  mode = readInt8();
  fps = readInt8();
  frameAmount = readInt16();
}

template<size_t N>
uint16_t Animator<N>::frame = 0;

template<size_t N>
uint16_t Animator<N>::repeat = 0;

template<size_t N>
uint8_t Animator<N>::readInt8() {
  return data.read();
}

template<size_t N>
uint16_t Animator<N>::readInt16() {
  ArrayToInteger16 converter;
  converter.bytes[0] = data.read();
  converter.bytes[1] = data.read();
  return converter.integer;
}