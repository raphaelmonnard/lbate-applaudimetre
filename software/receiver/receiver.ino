#include <SPI.h> // Not actually used but needed to compile
#include <avr/pgmspace.h>
#include "spectacle.h"
#include <RH_ASK.h>
#include <FastLED.h>

// #include "SSD1306Ascii.h"
// #include "SSD1306AsciiAvrI2c.h"

// // 0X3C+SA0 - 0x3C or 0x3D
// #define I2C_ADDRESS 0x3C

// // Define proper RST_PIN if required.
// #define RST_PIN -1

// SSD1306AsciiAvrI2c oled;

#define NUM_LEDS_CE 30
#define DATA_PIN_CE 7
#define COLOR_CE 0x0EA500
CRGB leds_CE[NUM_LEDS_CE];

#define NUM_LEDS_JB 30
#define DATA_PIN_JB 6
#define COLOR_JB 0x0045E5
CRGB leds_JB[NUM_LEDS_JB];

#define NUM_LEDS_LB 28
#define DATA_PIN_LB 5
#define COLOR_LB 0xD200AB
CRGB leds_LB[NUM_LEDS_LB];

#define NUM_LEDS_SA 30
#define DATA_PIN_SA 4
#define COLOR_SA 0xE5DD00
CRGB leds_SA[NUM_LEDS_SA];

struct DataPacket {
  int channel = 0;
  int value = 0;
  int micSensitivity = 0;
};

// Adafruit_SSD1306 display(128, 32, &Wire, -1);
RH_ASK driver;
DataPacket packet;
int channel = 0;
int value = 100;
int weight = 100;
int max = 100;
int mic = 0;
int micSensitivity = 1023;
int brightness = 32;
double temperature;

// void updateDisplay(const String& text) {
//   display.clearDisplay();
//   display.setCursor(0, 0);
//   display.println(text);
//   display.display();
// }

void setLEDs(CRGB* leds, int numLeds, int scaledValue, CRGB color) {
  for (int i = 0; i < numLeds; i++) {
    leds[i] = (i < scaledValue) ? color : CRGB::Black;
  }
}

template <uint8_t DATA_PIN, CRGB* leds, int numLeds>
void setupLeds() {
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, numLeds);
  pinMode(DATA_PIN, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  if (!driver.init())
    Serial.println("init failed");

  // #if RST_PIN >= 0
  //   oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
  // #else // RST_PIN >= 0
  //   oled.begin(&Adafruit128x32, I2C_ADDRESS);
  // #endif // RST_PIN >= 0
  //   // Call oled.setI2cClock(frequency) to change from the default frequency.

  // oled.setFont(Adafruit5x7);

  // oled.clear();

  // // first row
  // oled.println("set1X test");

  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for (;;) {}
  // }
  // display.display();
  // delay(500);
  // display.setTextSize(1);
  // display.setTextColor(SSD1306_WHITE);

  setupLeds<DATA_PIN_CE, leds_CE, NUM_LEDS_CE>();
  setupLeds<DATA_PIN_JB, leds_JB, NUM_LEDS_JB>();
  setupLeds<DATA_PIN_LB, leds_LB, NUM_LEDS_LB>();
  setupLeds<DATA_PIN_SA, leds_SA, NUM_LEDS_SA>(); 
}

void loop() {
  // updateDisplay("Spectacle");
  Serial.println("start loop");

  if (driver.available()) {
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen) && buflen == sizeof(packet)) {
      memcpy(&packet, buf, sizeof(packet));
      value = packet.value;
      channel = packet.channel;
      micSensitivity = packet.micSensitivity;
    }
  }

  int8_t value_CE = pgm_read_word(&(params[channel][0]));
  int8_t value_JB = pgm_read_word(&(params[channel][1]));
  int8_t value_LB = pgm_read_word(&(params[channel][2]));
  int8_t value_SA = pgm_read_word(&(params[channel][3]));
  int8_t weight = pgm_read_word(&(params[channel][4]));
  int8_t max = pgm_read_word(&(params[channel][5]));
  mic = map(analogRead(A0), 0, micSensitivity, 0, 100);

  int scaledValue = (weight * value / 100 + (100 - weight) * mic / 100) * max / 100;
  int scaledValue_CE = map(scaledValue, 0, 100, 0, NUM_LEDS_CE) * value_CE / 100;
  int scaledValue_JB = map(scaledValue, 0, 100, 0, NUM_LEDS_JB) * value_JB / 100;
  int scaledValue_LB = map(scaledValue, 0, 100, 0, NUM_LEDS_LB) * value_LB / 100;
  int scaledValue_SA = map(scaledValue, 0, 100, 0, NUM_LEDS_SA) * value_SA / 100;

  setLEDs(leds_CE, NUM_LEDS_CE, scaledValue_CE, COLOR_CE);
  setLEDs(leds_JB, NUM_LEDS_JB, scaledValue, COLOR_JB);
  setLEDs(leds_LB, NUM_LEDS_LB, scaledValue, COLOR_LB);
  setLEDs(leds_SA, NUM_LEDS_SA, scaledValue, COLOR_SA);

  brightness = map(analogRead(A1), 0, 1023, 0, 255);
  FastLED.setBrightness(brightness);

  FastLED.show();

  // update brightness for the red LED diodes, 
  // output PIN 3 (required to avoid conflict wiht RadioHead)
  // Analog sensor A2
  analogWrite(3, map(analogRead(A2), 0, 1023, 0, 255));
}
