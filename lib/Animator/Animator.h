#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>
#include <StandardCplusplus.h>
#include <map>

using namespace std;

template<size_t N>
class Animator {
  public:
    Animator(
      CRGB (&fastLeds)[N],
      uint16_t (*xyFunc)(uint8_t x, uint8_t y),
      File & file
    );
    static void loadAnimation(File & file);
    static void renderFrame();
  
  private:
    static CRGB (&leds)[N];
    static File & data;
    static uint16_t (*xy)(uint8_t x, uint8_t y);
    static void loadAnimationInfo();

    static uint8_t mode;
    static uint8_t fps;
    static uint16_t frameAmount;

    static std::map<uint16_t, CRGB> currentFrame;
    static uint16_t frame;
    static uint16_t repeat;

    static uint8_t readInt8();
    static uint16_t readInt16();
};
