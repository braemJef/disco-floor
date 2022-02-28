#include <Arduino.h>
#include <FastLED.h>
#include <SD.h>
#include <SPI.h>
#include <Animator.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// Screen messages
#define EMPTY_LINE "                    "
#define SCREEN_INITIALIZE_FAILED "Screen boot failed"
#define SCREEN_INITIALIZE_SUCCES "I) Screen booted"
#define MATRIX_INITIALIZE_TITLE "I) Init LEDs"
#define MATRIX_INITIALIZE_WIDTH "I)   Width: "
#define MATRIX_INITIALIZE_HEIGHT "I)   Height: "
#define SD_INITIALIZE_FAILED "E) FAILED MICRO SD"
#define SD_INITIALIZE_SUCCESS "I) Micro SD booted"
#define SD_OPENED_FILE "I)   F: "
#define WAITING_FOR_RESTART "I) Please restart..."
#define STARTUP_SUCCESS "I) Successfull boot!"

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

// I2C Screen config
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint16_t lines = 0;
void displayprintln(String str) {
  display.setCursor(0, (lines % 8) * 8);
  display.print(EMPTY_LINE);
  display.setCursor(0, (lines % 8) * 8);
  display.print(str);
  display.display();
  if (lines > 7) {
    display.ssd1306_command(SSD1306_SETSTARTLINE | ((lines % 8) + 1) * 8);
  }
  lines++;
}

// set up variables using the SD utility library functions:
File root;
File myFile;
const int chipSelect = 4;

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
uint8_t fps = 1;
uint32_t lastRender = 0;

void setup() {
  bool ready = true;
  // Only in development
  // Initialize serial connection
  Serial.begin(9600);
  Serial.println("Serial connected.");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(SCREEN_INITIALIZE_FAILED);
    ready = false;
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE, 0);
    displayprintln(SCREEN_INITIALIZE_SUCCES);
  }

  // Initialize button to cycle animations
  pinMode(BUTTON_PIN, INPUT);

  // Initialize LED array
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(16);
  FastLED.setCorrection(TypicalLEDStrip);

  displayprintln(MATRIX_INITIALIZE_TITLE);
  displayprintln((String)MATRIX_INITIALIZE_WIDTH + NUM_COLUMNS);
  displayprintln((String)MATRIX_INITIALIZE_HEIGHT + NUM_ROWS);

  // Initialize root folder
  SD.begin(4);
  root = SD.open(F("ANIM"));
  myFile = root.openNextFile();
  if (!myFile) {
    ready = false;
    displayprintln(SD_INITIALIZE_FAILED);
  } else {
    displayprintln(SD_INITIALIZE_SUCCESS);
    displayprintln((String)SD_OPENED_FILE + myFile.name());
  }

  animator = Animator(XY);
  animator.loadAnimation(myFile);
  fps = animator.getFps();

  // If any fatal error occured, stop any execution.
  if (!ready) {
    displayprintln(WAITING_FOR_RESTART);
    while (true) {
      delay(1000);
    }
  }
  displayprintln(STARTUP_SUCCESS);
  delay(1000);
  displayprintln("I) 3...");
  delay(1000);
  displayprintln("I) 2...");
  delay(1000);
  displayprintln("I) 1...");
  delay(1000);
  display.clearDisplay();
  display.ssd1306_command(SSD1306_SETSTARTLINE | 0);
}

void loop() {
  EVERY_N_SECONDS(2) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("====== STATUS ======");
    display.println("");
    display.println((String)"FPS: " + FastLED.getFPS() + "/" + animator.getFps());
    display.println((String)"BRIGHTNESS: " + map(FastLED.getBrightness(), 8, 204, 1, 100) + "%");
    display.println((String)"MODE: Animate");
    display.println((String)"FILE: " + myFile.name());
    display.display();
  }

  if (millis() > lastRender + (1000 / fps)) {
    lastRender = millis();
    FastLED.show();
    animator.renderFrame(leds, NUM_LEDS, myFile);
  }

  // Check for button press and potentiometer change very 16ms
  EVERY_N_MILLISECONDS(16) {
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
      }
      animator.loadAnimation(myFile);
      fps = animator.getFps();
    }

    int sensorValue = analogRead(POTENTIOMETER_PIN);
    int brightness = map(sensorValue, 0, 1023, 8, 204);
    FastLED.setBrightness(brightness);
  }
}