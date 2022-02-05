#include <FastLED.h>
#include <SD.h>

typedef struct {
  uint16_t xy;
  CRGB color;
} pixelDictionary;

class AnimatorSimple {
  public:
    AnimatorSimple();
    AnimatorSimple(char fileName[]);
    AnimatorSimple(uint16_t (*xyFunc)(uint8_t x, uint8_t y));

    void loadAnimation(char fileName[]);
    void loadAnimationInfo();
    void renderFrame(CRGB *leds, uint16_t numLeds);

    File file;
    uint16_t (*xy)(uint8_t x, uint8_t y);
    uint8_t mode;
    uint8_t fps;
    uint16_t frameAmount;

    uint16_t frame;
    uint16_t framePixelAmount;
};