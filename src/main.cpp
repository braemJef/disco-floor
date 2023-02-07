#include <Arduino.h>

#include <FastLED.h>
#include <SD.h>
#include <SPI.h>
#include <Animator.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// For enable/disable Serial printing
// #define Sprintln(a) (Serial.println(a))
#define Sprintln(a) 

// User input config
#define KNOB_BRIGHTNESS_PIN A0
#define BUTTON_CYCLE_ANIM_PIN 6
#define BUTTON_MODE_SELECT_PIN 7
int prevButtonCycleAnimState = HIGH;
int prevButtonModeSelectState = HIGH;
uint8_t currentMode = 0; // 0 = cycle, 1 = select
#define CYCLE_DURATION 30 // Cycle duration in seconds
unsigned long newCycleMillis = 0;

// SSD1306 config
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Led matrix config
#define NUM_ROWS 16
#define NUM_COLUMNS 32
#define NUM_LEDS (NUM_ROWS * NUM_COLUMNS)
#define DATA_PIN 0
#define MIN_BRIGHTNESS 12
#define MAX_BRIGHTNESS 255
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
  Sprintln("Serial connected.");

  // Initialize LCD screen
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Sprintln(F("Initialization SSD1306 failed."));
    for(;;);
  }
  Sprintln("Initialized SSD1306 lcd.");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(22, 24);
  display.println("Booting");
  display.display();

  // Initialize button to cycle animations
  pinMode(BUTTON_CYCLE_ANIM_PIN, INPUT);

  // Initialize button to switch modes
  pinMode(BUTTON_MODE_SELECT_PIN, INPUT);

  // Initialize FastLED
  FastLED.addLeds<WS2812B, DATA_PIN, EOrder::GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(5);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.clear();
  FastLED.show();
  Sprintln("Initialized FastLED.");
  
  // Initialize root folder
  if (!SD.begin(SD_CHIP_SELECT)) {
    Sprintln("SD initialization failed.");
    while (1);
  }

  root = SD.open("ANIM");
  myFile = root.openNextFile();
  if (!myFile) {
      Sprintln("Could not open first file.");
  } else {
    Sprintln((String)"Opened first file: " + myFile.name());
  }

  animator = Animator(XY);
  animator.loadAnimation(myFile);
  fps = animator.fps;

  display.setTextSize(1);
  display.clearDisplay();
  display.display();
}

void draw() {
  unsigned long currentMillis = millis();
  if (currentMillis > newMillis) {
    newMillis = currentMillis + (1000 / fps);
    animator.renderFrame(leds, NUM_LEDS, myFile);
    FastLED.show();
  }
}

void openNextAnimation() {
  myFile = root.openNextFile();
  if (!myFile) {
    root.rewindDirectory();
    myFile = root.openNextFile();
    Sprintln((String)"Could not open next file, rewind directory.");
  }
  animator.loadAnimation(myFile);
  fps = animator.fps;
  FastLED.clear(true);
  FastLED.show();
  Sprintln((String)"Opened next file: " + myFile.name());
}

void checkMode() {
  if (currentMode = 0) {
    unsigned long currentCycleMillis = millis();
    if (currentCycleMillis > newCycleMillis && animator.cycle >= 1) {
      newCycleMillis = currentCycleMillis + (CYCLE_DURATION * 1000);
      openNextAnimation();
    }
  }
}

void cycleAnimButton() {
  EVERY_N_MILLISECONDS(1000 / 30) {
    int newButtonState = digitalRead(BUTTON_CYCLE_ANIM_PIN);

    if (newButtonState == LOW) {
      prevButtonCycleAnimState = LOW;
    }

    if (newButtonState == HIGH && prevButtonCycleAnimState == LOW) {
      prevButtonCycleAnimState = HIGH;
      openNextAnimation();
    }
  }
}

void modeSelectButton() {
  EVERY_N_MILLISECONDS(1000 / 30) {
    int newButtonState = digitalRead(BUTTON_MODE_SELECT_PIN);

    if (newButtonState == LOW) {
      prevButtonModeSelectState = LOW;
    }

    if (newButtonState == HIGH && prevButtonModeSelectState == LOW) {
      prevButtonModeSelectState = HIGH;
      if (currentMode == 1) {
        currentMode = 0;
        Sprintln((String)"Changed mode to CYCLE");
      } else {
        currentMode = 1;
        Sprintln((String)"Changed mode to SELECT");
      }
    }
  }
}

void brightnessKnob() {
  EVERY_N_MILLIS(1000 / 30) {
    int sensorValue = analogRead(KNOB_BRIGHTNESS_PIN);
    int brightness = map(sensorValue, 0, 1023, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    if (abs(brightness - FastLED.getBrightness()) > 2 || brightness == MAX_BRIGHTNESS || brightness == MIN_BRIGHTNESS) {
      FastLED.setBrightness(brightness);
    }
  }
}

void lcd() {
  EVERY_N_MILLIS(1000 / 10) {
    int currentFps = FastLED.getFPS();
    int currentBrightness = FastLED.getBrightness();
    int realBrightness = map(currentBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 1, 100);
    String currentFile = myFile.name();
    String modeText = currentMode == 0 ? "cycling" : "select";

    display.clearDisplay();

    display.drawLine(0, 12, 128, 12, SSD1306_WHITE);
    display.drawLine(0, 52, 128, 52, SSD1306_WHITE);

    display.setCursor(2, 2);
    display.print((String) "fps " + currentFps);

    String brightnessText = (String) realBrightness + "%";
    display.setCursor(128 - 2 - (brightnessText.length() * 8), 2);
    display.print(brightnessText);

    display.setCursor(2, 54);
    display.print(modeText);

    display.setCursor(0, 16);
    display.print(currentFile);

    display.display();
  }
}

void loop() {
  draw();
  lcd();
  cycleAnimButton();
  modeSelectButton();
  checkMode();
  brightnessKnob();
}
