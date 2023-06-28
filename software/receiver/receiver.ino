#include <SPI.h> // Not actualy used but needed to compile
#include <avr/pgmspace.h>
#include "spectacle.h"

// Init RadioHead for transmission
#include <RH_ASK.h>
RH_ASK driver;

// Init display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Init FastLED
#include<FastLED.h>

#define NUM_LEDS_CE 20
#define DATA_PIN_CE 7
CRGB leds_CE[NUM_LEDS_CE];

#define NUM_LEDS_JB 20
#define DATA_PIN_JB 6
CRGB leds_JB[NUM_LEDS_JB];

#define NUM_LEDS_LB 20
#define DATA_PIN_LB 5
CRGB leds_LB[NUM_LEDS_LB];

#define NUM_LEDS_SA 20
#define DATA_PIN_SA 4
CRGB leds_SA[NUM_LEDS_SA];

struct DataPacket {
  int channel = 0;
  int value = 0;
  int micSensitivity = 0;
};

int scaledValue;
int scaledValue_CE = 1;
int channel = 0; 
int value = 100;
int value_CE = 100;
int weight = 100;
int max = 100;
int mic = 0;
int micSensitivity = 1023;

int brightness = 32;

void updateDisplay(String text) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(text);
  display.display();  
}

void displayPacket(DataPacket packet) {
  Serial.print("Received Packet - channel: ");
  Serial.print(packet.channel);
  Serial.print(", Value: ");
  Serial.println(packet.value);
}

void setLEDs(CRGB* leds, int numLeds, int scaledValue, CRGB color) {
  for (int i = 0; i < numLeds; i++) {
    if (i < scaledValue) {
      leds[i] = color;
    } else {
      leds[i] = CRGB::Black;
    }
  }
}

template <uint8_t DATA_PIN>
void setupLeds(CRGB* leds, int numLeds) {
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, numLeds);
  pinMode(DATA_PIN, OUTPUT);
}

void setup()
{
  Serial.begin(9600);	// Debugging only
  if (!driver.init())
        Serial.println("init failed");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);    

  setupLeds<DATA_PIN_CE>(leds_CE, NUM_LEDS_CE);
  setupLeds<DATA_PIN_JB>(leds_JB, NUM_LEDS_JB);
  setupLeds<DATA_PIN_LB>(leds_LB, NUM_LEDS_LB);
  setupLeds<DATA_PIN_SA>(leds_SA, NUM_LEDS_SA);
}

void loop()
{
  updateDisplay("Spectacle");

  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  DataPacket packet;
  if (driver.available()) {
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen)) {
      Serial.println("Message received !");
      // Message received
      if (buflen == sizeof(packet)) {
        memcpy(&packet, buf, sizeof(packet));
        // Process the received packet
        value = packet.value;
        channel = packet.channel;
        micSensitivity = packet.micSensitivity;
        //displayPacket(packet);
      }
    }
  }
  
  int8_t value_CE = pgm_read_word(&(params[channel][0]));
  int8_t weight = pgm_read_word(&(params[channel][4]));
  int8_t max = pgm_read_word(&(params[channel][5]));
  mic = map(analogRead(A0), 0, micSensitivity, 0, 100);

  scaledValue = (weight * value / 100 + (100 - weight) * mic / 100 ) * max / 100; // valeur relative
  scaledValue_CE = map(scaledValue, 0, 100, 0, NUM_LEDS_CE) * value_CE / 100; // valeur absolue

  // Serial.print("Channel: ");
  // Serial.print(channel);
  // Serial.print(" - weight: ");
  // Serial.print(weight);
  // Serial.print(" - value: ");
  // Serial.print(value);
  // Serial.print(" - mic: ");
  // Serial.print(mic);
  // Serial.print(" - scaledValue: ");
  // Serial.print(scaledValue);
  // Serial.print(" - scaledValue CE: ");
  // Serial.println(scaledValue_CE);

  setLEDs(leds_CE, NUM_LEDS_CE, scaledValue_CE, CRGB::Green);
  setLEDs(leds_JB, NUM_LEDS_JB, scaledValue, CRGB::Blue);
  setLEDs(leds_LB, NUM_LEDS_LB, scaledValue, CRGB::Red);
  setLEDs(leds_SA, NUM_LEDS_SA, scaledValue, CRGB::Yellow);

  brightness = map(analogRead(A1), 0, 1023, 0, 255);
  FastLED.setBrightness(brightness); // Set the initial brightness  
  
  FastLED.show();
  }
